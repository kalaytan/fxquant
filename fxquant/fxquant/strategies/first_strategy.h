#pragma once
#include "ta.h"
#include "types.h"
#include "bar_data.h"
#include "strategy.h"

namespace fx {

class first_strategy : public strategy
{
public:
    struct params
    {
        double volume;
        int sl;
        int tp;
        int min_candle;
        int lines_dist;
        int max_trades_per_candle;
    };

    first_strategy(const params& par) :
        params_(par)
        , total_trades_(0)
        , test_counter_(0)
        , buy_line_(0)
        , sell_line_(0)
        , trade_opens_in_current_bar_(0)
    {
        if (params_.volume < 0.01
            || params_.sl < 0
            || params_.tp < 0
            || params_.min_candle <= 0
            || params_.lines_dist <= 0
            || params_.max_trades_per_candle <= 0
            )
        {
            throw std::invalid_argument("Incoming invalid param ");
        }
    }

    const params& get_params() const
    {
        return params_;
    }
    virtual std::string get_json_params() const override;
private:
    virtual void on_tick(const tick_data& tick) override;
    virtual void on_bar(timeframe_type time_frame, const bar_data& bar) override;
    bool get_bars(timeframe_type tf, int many, bar_array_type& latest_bars) const;
    bool check_orders_closing(const tick_data& tick);
    bool is_bar_legal(int bar, const bar_array_type& latest_bars) const;
    bool check_trades(const tick_data& tick);

    template <class T>
    bool order_send(const tick_data& tick, double open_price, int sl, int tp)
    { //todo вынести volume v входсящие 
        try
        {
            order_ptr optr = std::make_shared<T>
                (engine_ptr_->get_symbol(), params_.volume
                    , open_price, sl, tp);
            optr->open(tick);
            get_opened_orders().push_back(optr);
        }
        catch (const std::exception&)
        {
            DEBUG_TRACE("Failed to create order");
            return false;
        }

        return true;
    }
private:
    params params_;
    unsigned int total_trades_;
    unsigned int test_counter_;
    int trade_opens_in_current_bar_;
    double buy_line_;
    double sell_line_;
};
} // namespace fx
