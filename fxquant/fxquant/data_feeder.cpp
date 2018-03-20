#include <cmath>
#include <chrono>
#include "data_feeder.h"

using namespace std::placeholders;
using namespace std::chrono_literals;

namespace fx {

data_feeder::data_feeder(symbol sym) : symbol_(sym), precision_(static_cast<int>(log10(1 / symbol_pip(sym))))
{
    // create the candle factories for all time frames
    const timeframe_type tf_arr[] = { 1min, 5min, 15min, 30min, 1h, 4h, 168h, 720h };

    for (auto tf : tf_arr)
    {
        candle_factories_.insert({ tf, candle_factory(tf,
            std::bind(&data_feeder::on_bar, this, tf, _1)) });
    }

    DEBUG_ENSURE((precision_ == 5) || (precision_ == 3));
}

bool data_feeder::add_callback(data_callback_ptr cb_ptr)
{
    if (cb_ptr)
    {
        std::lock_guard<std::mutex> lock(lock_);
        auto r = callbacks_.insert(cb_ptr);
        return r.second;
    }

    return false;
}

bool data_feeder::remove_callback(data_callback_ptr cb_ptr)
{
    if (cb_ptr)
    {
        std::lock_guard<std::mutex> lock(lock_);
        auto it = callbacks_.find(cb_ptr);

        if (it != callbacks_.end())
        {
            callbacks_.erase(it);
            return true;
        }
    }

    return false;
}

void data_feeder::on_tick(const tick_data& tick, bool ignore_callback)
{
    for (auto& fac : candle_factories_)
    {
        fac.second.put_tick(tick);
    }

    if (!ignore_callback)
    {
        std::lock_guard<std::mutex> lock(lock_);

        for (auto cb_ptr : callbacks_)
        {
            cb_ptr->on_tick(tick);
        }
    }
}

void data_feeder::on_bar(timeframe_type tf, const bar_data& bar)
{
    std::lock_guard<std::mutex> lock(lock_);

    for (auto cb_ptr : callbacks_)
    {
        cb_ptr->on_bar(tf, bar);
    }
}

double data_feeder::normalize(double d) const
{
    return (precision_ == 5) ? (double)fixed_point<5>(d) : (double)fixed_point<3>(d);
}

} // namespace fx