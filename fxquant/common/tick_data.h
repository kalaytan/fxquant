#pragma once
#include <chrono>
#include "types.h"
#include "debug.h"

namespace fx {

class tick_data
{
public:
    tick_data(double bid = 0.0, double ask = 0.0,
        timepoint_type time = std::chrono::system_clock::now()) :
        bid_(bid), ask_(ask), time_(time)
    {
        DEBUG_ASSERT((bid_ == 0 && ask_ == 0) || (ask_ > bid_));
    }

    double get_bid() const
    {
        return bid_;
    }

    double get_ask() const
    {
        return ask_;
    }

    double get_mid_price() const
    {
        return bid_ + (ask_ - bid_) / 2;
    }

    double get_spread() const
    {
        return ask_ - bid_;
    }

    timepoint_type get_time() const
    {
        return time_;
    }

    bool operator <(const tick_data& rhs) const
    {
        return (time_ < rhs.time_);
    }

private:
    double bid_;
    double ask_;
    timepoint_type time_;
};

} // namespace fx
