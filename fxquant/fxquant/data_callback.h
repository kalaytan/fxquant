#pragma once
#include <memory>
#include "types.h"
#include "bar_data.h"
#include "tick_data.h"

namespace fx {

struct data_callback
{
    virtual void on_tick(const tick_data& tick) = 0;
    virtual void on_bar(timeframe_type tf, const bar_data& bar) = 0;

    virtual ~data_callback() = default;
};

typedef std::shared_ptr<data_callback> data_callback_ptr;

} // namespace fx
