#include <ctime>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <bsoncxx/json.hpp>
#include <bsoncxx/exception/exception.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include "debug.h"
#include "config.h"
#include "logger.h"
#include "tick_data.h"
#include "controlled_feeder.h"
#include "gui_server.h"
#include "fixed_point.h"

#if defined(_WIN32)
#define timegm _mkgmtime
#endif

using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::open_document;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::basic::kvp;

using namespace std::chrono;

namespace {

time_t utc_time(int day, int month, int year)
{
    time_t tt;
    time(&tt);
    tm* ptm = gmtime(&tt);

    ptm->tm_mday = day;
    ptm->tm_mon = month - 1;
    ptm->tm_year = year - 1900;
    ptm->tm_hour = 0;
    ptm->tm_min = 0;
    ptm->tm_sec = 0;

    return timegm(ptm);
}
}

namespace fx {

controlled_feeder::controlled_feeder() :
    data_feeder(symbol_from_string(config::get_string("controlled_feeder", "symbol")))
    , day_(config::get_int("controlled_feeder", "start_day"))
    , month_(config::get_int("controlled_feeder", "start_month"))
    , year_(config::get_int("controlled_feeder", "start_year"))
    , days_(config::get_int("controlled_feeder", "calculate_days"))
    , use_cache_(config::get_bool("controlled_feeder", "use_cache"))
    , lookback_minutes_(config::get_int("controlled_feeder", "lookback_minutes"))
    , speed_factor_(config::get_int("controlled_feeder", "speed_factor"))
    , running_(false), delay_(0), bar_count_(0)
{
    if (speed_factor_ <= 0)
    {
        speed_factor_ = 1;
    }

    if (lookback_minutes_ <= 0)
    {
        lookback_minutes_ = 0;
    }

    if (get_symbol() == symbol::undefined)
    {
        throw std::runtime_error("Symbol is undefined.");
    }
}

void controlled_feeder::sleep(const tick_data& td)
{
    if (!is_warming_up())
    {
        const int min_delay = 20; // ms
        gui_server::options opt;
        int speed_factor = speed_factor_;

        if (gui_server::instance().get_options(symbol_, opt) && (opt.speed_factor_ > 0))
        {
            speed_factor = opt.speed_factor_;
        }

        if (prev_time_.time_since_epoch().count() > 0)
        {
            auto ms = duration_cast<milliseconds>(td.get_time() - prev_time_).count();

            if (ms <= 60000) // 1min
            {
                delay_ += static_cast<int>(ms);
            }
        }

        if (delay_ >= (min_delay * speed_factor))
        {
            int d = static_cast<int>(delay_ / speed_factor);
            std::this_thread::sleep_for(milliseconds(d));
            delay_ -= (d * speed_factor);
            //DEBUG_TRACE("delay=%d speed_factor=%d ds=%d", d, speed_factor, d * speed_factor);
        }
    }

    prev_time_ = td.get_time();
}

void controlled_feeder::on_bar(timeframe_type tf, const bar_data& bar)
{
    if (tf.count() == 1)
    {
        bar_count_++;
        //DEBUG_TRACE("bar=%d", (int)bar_count_);
    }

    data_feeder::on_bar(tf, bar);
}

void controlled_feeder::run()
{
    DEBUG_ASSERT(cursor_ptr_);

    try
    {
        if (use_cache_ && !cache_.empty())
        {
            for (const auto& td : cache_)
            {
                if (!running_)
                {
                    break;
                }

                on_tick(td, is_warming_up());              
                sleep(td);
            }
        }
        else
        {
            cache_.clear();

            for (auto& doc : *cursor_ptr_)
            {
                if (!running_)
                {
                    break;
                }

                tick_data td
                {
                    normalize(doc["bid"].get_double()),
                    normalize(doc["ask"].get_double()),
                    system_clock::time_point(doc["_id"].get_date().value)
                };

                if (use_cache_)
                {
                    cache_.push_back(td);
                }

                on_tick(td, is_warming_up());
                sleep(td);
            }
        }
    }
    catch (...)
    {
    }

    stop_event_.signal();
}

bool controlled_feeder::start()
{
    if (running_)
    {
        return false; // already started
    }

    try
    {
        if (cache_.empty())
        {
            const std::string collection_name = symbol_to_string(get_symbol()) + "_ticks";
            auto client = mongodb::instance().get_client();
            auto collection = (*client)["prices-data"][collection_name];

            auto from_time = std::chrono::system_clock::from_time_t(utc_time(day_, month_, year_));

            if (lookback_minutes_ > 0)
            {
                from_time -= minutes(lookback_minutes_);
            }

            auto to_time = from_time + hours(24 * days_);

            auto filter = document{}
                << "_id"
                << open_document
                << "$gte" << bsoncxx::types::b_date{ from_time }
                << "$lt" << bsoncxx::types::b_date{ to_time }
            << close_document << finalize;

            //auto order = document{} << "_id" << 1 << finalize;
            auto opts = mongocxx::options::find{};
            //opts.sort(order.view());

            const size_t count = static_cast<size_t>(collection.count(filter.view()));
            cache_.reserve(count);

            auto cursor = collection.find(filter.view(), opts);
            cursor_ptr_ = std::make_unique<mongocxx::cursor>(std::move(cursor));
            logger::instance().info("Data loaded from DB.");
        }

        running_ = true;
        thread_ptr_ = std::make_unique<std::thread>(std::bind(&controlled_feeder::run, this));

        return true;
    }
    catch (const std::exception& x)
    {
        DEBUG_TRACE("controlled_feeder::start(): %s", x.what());
    }
    catch (...)
    {
        DEBUG_TRACE("controlled_feeder::start(): unknown exception");
    }

    return false;
}

bool controlled_feeder::stop()
{
    if (!running_)
    {
        return false; // not started
    }

    running_ = false;
    thread_ptr_->join();
    return true;
}

} // namespace fx
