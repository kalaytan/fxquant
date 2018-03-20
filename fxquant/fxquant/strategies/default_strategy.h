#pragma once
#include "ta.h"
#include "types.h"
#include "bar_data.h"
#include "strategy.h"

namespace fx {

class default_strategy : public strategy
{
public:
    struct param_type
    {
        unsigned int ma_period_;
        ma_algo ma_algo_;
        bar_field field_;
    };

    default_strategy(const param_type& par) : params_(par) {}

    virtual std::string get_json_params() const override;

private:
    virtual void on_tick(const tick_data& tick) override;
    virtual void on_bar(timeframe_type time_frame, const bar_data& bar) override;

private:
    param_type params_;
};

} // namespace fx
