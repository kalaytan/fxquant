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
#include "dummy_feeder.h"

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

dummy_feeder::dummy_feeder() :
    data_feeder(symbol_from_string(config::get_string("d_feeder", "symbol")))
    , day_(config::get_int("d_feeder", "start_day"))
    , month_(config::get_int("d_feeder", "start_month"))
    , year_(config::get_int("d_feeder", "start_year"))
    , days_(config::get_int("d_feeder", "calculate_days"))
    , use_cache_(config::get_bool("d_feeder", "use_cache")),
    running_(false)
{
    if (get_symbol() == symbol::undefined)
    {
        throw std::runtime_error("Symbol is undefined.");
    }
}

void dummy_feeder::run()
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

                on_tick(td);
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

                on_tick(td);
                //std::this_thread::sleep_for(10ms); // short delay
            }
        }
    }
    catch (...)
    {
    }

    stop_event_.signal();
}

bool dummy_feeder::start()
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
        thread_ptr_ = std::make_unique<std::thread>(std::bind(&dummy_feeder::run, this));

        return true;
    }
    catch (const std::exception& x)
    {
        DEBUG_TRACE("dummy_feeder::start(): %s", x.what());
    }
    catch (...)
    {
        DEBUG_TRACE("dummy_feeder::start(): unknown exception");
    }

    return false;
}

bool dummy_feeder::stop()
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
