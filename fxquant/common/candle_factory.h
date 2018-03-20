#pragma once
#include <functional>
#include "bar_data.h"
#include "tick_data.h"

namespace fx {

class candle_factory
{
public:
    typedef std::function<void(const bar_data&)> callback_func;

public:
    candle_factory(timeframe_type time_frame, callback_func on_bar);
    void put_tick(const tick_data& tick);

    timeframe_type get_time_frame() const
    {
        return time_frame_;
    }

private:
    const timeframe_type time_frame_;
    const unsigned int divider_;
    callback_func on_bar_;
    bar_data bar_;
};

} // namespace fx
