#pragma once
#include "strategy_generator.h"

namespace fx {

    class first_strategy_generator : public strategy_generator
    {
    public:
        first_strategy_generator();

        void reset() override;
        strategy_ptr create() override;

    private:
        const int min_candle_start_;
        const int min_candle_end_;
        const int min_candle_step_;
        int current_min_candle_;

        const int max_trades_per_candle_start_;
        const int max_trades_per_candle_end_;
        const int max_trades_per_candle_step_;
        int current_max_trades_per_candle_;

    };

} // namespace fx
