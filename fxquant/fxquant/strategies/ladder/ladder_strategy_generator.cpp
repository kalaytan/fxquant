#include "config.h"
#include "ladder_strategy_generator.h"
#include "ladder_strategy.h"

namespace fx {

ladder_strategy_generator::ladder_strategy_generator() :
    min_candle_start_(config::get_int("optimizer", "min_candle_start"))
    , min_candle_end_(config::get_int("optimizer", "min_candle_end"))
    , min_candle_step_(config::get_int("optimizer", "min_candle_step"))
    , current_min_candle_(min_candle_start_)
    , max_trades_per_candle_start_(config::get_int("optimizer", "max_trades_per_candle_start"))
    , max_trades_per_candle_end_(config::get_int("optimizer", "max_trades_per_candle_end"))
    , max_trades_per_candle_step_(config::get_int("optimizer", "max_trades_per_candle_step"))
    , current_max_trades_per_candle_(max_trades_per_candle_start_) {}

void ladder_strategy_generator::reset()
{
    current_min_candle_ = min_candle_start_;
    current_max_trades_per_candle_ = max_trades_per_candle_start_;
}

strategy_ptr ladder_strategy_generator::create()
{
    /*struct params
    {
        double volume;
        int sl;
        int tp;
        int min_candle; <<<<<
        int lines_dist;
        int max_trades_per_candle;<<<<<
    };*/
    if (current_min_candle_ > min_candle_end_)
    {
        return nullptr;
    }
    ladder_strategy::params par{
        config::get_double("params","volume")
        , config::get_int("params","sl")
        , config::get_int("params","tp")
        , current_min_candle_
        , config::get_int("params","lines_dist")
        , current_max_trades_per_candle_
    };
    DEBUG_TRACE("Optimizer runnin new settings:min_candle = %d, max_trades = %d"
        , current_min_candle_, current_max_trades_per_candle_);

    current_max_trades_per_candle_ += max_trades_per_candle_step_;

    //// run last parameter of optimizer
    //if (current_max_trades_per_candle_ > max_trades_per_candle_end_
    //    &&current_max_trades_per_candle_ != max_trades_per_candle_end_)
    //{
    //    current_max_trades_per_candle_ = max_trades_per_candle_end_;
    //}

    //// run last parameter of optimizer
    //if (current_min_candle_ + max_trades_per_candle_step_ > min_candle_end_
    //    && current_min_candle_ != current_min_candle_)
    //{
    //    current_min_candle_ = min_candle_end_;
    //}

    if (current_max_trades_per_candle_ > max_trades_per_candle_end_)
    {
        current_max_trades_per_candle_ = max_trades_per_candle_start_;
        current_min_candle_ += min_candle_step_;
    }

    return std::make_shared<ladder_strategy>(par);
}

} // namespace fx
