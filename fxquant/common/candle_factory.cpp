#include "candle_factory.h"

using namespace std::chrono;

namespace fx {

candle_factory::candle_factory(timeframe_type time_frame, callback_func on_bar) :
    time_frame_(time_frame), divider_(time_frame.count() * 60), on_bar_(on_bar),
    bar_{ 0.0, 0.0, 0.0, 0.0, 0 }
{
}

void candle_factory::put_tick(const tick_data& tick)
{
    time_t t = system_clock::to_time_t(tick.get_time());
    time_t tick_time = static_cast<time_t>((t / divider_) * divider_);

    double bid = tick.get_bid();

    if (tick_time != bar_.t)
    {
        // new candle
        if (bar_.t > 0)
        {
            on_bar_(bar_);
        }

        bar_.o = bid;
        bar_.h = bid;
        bar_.l = bid;
        bar_.c = bid;
        bar_.t = tick_time;
    }
    else
    {
        bar_.c = bid;

        if (bid > bar_.h)
        {
            bar_.h = bid;
        }
        else if (bid < bar_.l)
        {
            bar_.l = bid;
        }
    }
}
} // namespace fx
