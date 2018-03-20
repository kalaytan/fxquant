#pragma once
#include "data_feeder.h"
#include "strategy_generator.h"

namespace fx {

class optimizer
{
public:
    typedef std::function<void(strategy_ptr)> callback_func;

public:
    optimizer(data_feeder_ptr df_ptr, strategy_generator_ptr sg_ptr,
        callback_func on_start = nullptr, callback_func on_stop = nullptr);
    bool run();

private:
    data_feeder_ptr df_ptr_;
    strategy_generator_ptr sg_ptr_;
    callback_func on_start_;
    callback_func on_stop_;
};

} // namespace fx
