#pragma once
#include "strategy_generator.h"

namespace fx {

class default_strategy_generator : public strategy_generator
{
public:
    default_strategy_generator();

    void reset() override;
    strategy_ptr create() override;

private:
    const std::vector<ma_algo> algos_;
    const unsigned int period_start_;
    const unsigned int period_end_;
    const unsigned int period_step_;

    size_t cur_algo_index_;
    unsigned int cur_period_;
};

} // namespace fx
