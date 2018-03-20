#include "default_strategy_generator.h"

namespace fx {

    default_strategy_generator::default_strategy_generator() :
        algos_{ ma_algo::sma, ma_algo::ema }, period_start_(20), period_end_(30), period_step_(10),
        cur_algo_index_(0), cur_period_(period_start_)
    {
    }

    void default_strategy_generator::reset()
    {
        cur_algo_index_ = 0;
        cur_period_ = period_start_;
    }

    strategy_ptr default_strategy_generator::create()
    {
        if (cur_period_ > period_end_)
        {
            cur_period_ = period_start_;
            cur_algo_index_++;
        }

        if (cur_algo_index_ >= algos_.size())
        {
            return nullptr;
        }

        default_strategy::param_type param{ cur_period_, algos_[cur_algo_index_], bar_field::c };
        cur_period_ += period_step_;

        return std::make_shared<default_strategy>(param);
    }

} // namespace fx
