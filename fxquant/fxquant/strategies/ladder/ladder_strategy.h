#pragma once
#include "ta.h"
#include "types.h"
#include "debug.h"
#include "bar_data.h"
#include "strategy.h"
#include "fixed_point.h"
#include <map>

namespace fx {
class ladder_strategy;

class ladder_strategy_compute
{

public:
    ladder_strategy_compute(const ladder_strategy& strategy);
    double get_range_min() const
    {
        DEBUG_REQUIRE(range_min_ != undefined_value<double>() && range_min_ > 0);
        return range_min_;
    }
    double get_range_max() const
    {
        DEBUG_REQUIRE(range_max_ != undefined_value<double>() && range_max_ > 0);
        return range_max_;
    }
    double get_range() const
    {
        DEBUG_REQUIRE(range_ != undefined_value<double>() && range_ > 0);
        return range_;
    }
    int get_section() const
    {
        DEBUG_REQUIRE(section_ != undefined_value<int>());
        return section_;
    }
    double get_dynamic_section_risk() const
    {
        DEBUG_REQUIRE(range_ != undefined_value<double>());
        return dynamic_section_risk_;
    }
    double get_nearest_level() const
    {
        DEBUG_ENSURE(nearest_level_ > 0);
        return nearest_level_;
    }
    double get_lots_exposure() const
    {
        return lots_exposure_;
    }
    void initialize(const tick_data& tick);

    bool is_top_half() const
    {
        return is_top_half_;
    }

    double get_equity() const
    {
        return equity_;
    }

    double get_close_profit() const
    {
        return close_profit_;
    }

    unsigned int get_open_buy_trades() const
    {
        return open_buy_trades_;
    }

    unsigned int get_open_sell_trades() const
    {
        return open_sell_trades_;
    }

    double get_open_buy_volume() const
    {
        return open_buy_volume_;
    }

    double get_open_sell_volume() const
    {
        return open_sell_volume_;
    }

private:
    void reset_values(const tick_data& tick);

    void set_profits(const tick_data& tick);
    // highest and lowest order.open_price from all open orders list
    void set_open_trades_minmax_and_range(const tick_data& tick);
    void set_top_half(const tick_data& tick);
    void set_section_i_am_in(const tick_data& tick);
    void set_dynamic_section_risk(const tick_data& tick);
    void set_range(const tick_data& tick);
    void set_nearest_level(const tick_data& tick);
    void loop_opened_orders();
    void loop_closed_orders();
private:
    const ladder_strategy& strategy_;
    double range_min_;
    double range_max_;
    double range_;
    bool is_top_half_;
    double mid_price_;
    int section_;
    double dynamic_section_risk_;
    double nearest_level_;
    double lots_exposure_;
    double equity_;
    double close_profit_;
    unsigned int open_buy_trades_;
    unsigned int open_sell_trades_;
    double open_buy_volume_;
    double open_sell_volume_;
};

class ladder_strategy : public strategy
{
public:
    typedef base_order::custom_data_ptr custom_data_ptr;
    struct params
    {
        double volume;
        int sl;
        int tp;
        int soft_tp;
        int step;
        int lvl_tolerance;
        int trades_per_lvl;
        double close_cycle_on_profit;
        int sections_in_half_range;
        int sections_offset;
        double max_lots_allowed;
        int max_spread;
        int plr;
    };

    ladder_strategy();

    ~ladder_strategy();

    ladder_strategy(const params& par);

    void print_spread() const;

    const params& get_params() const
    {
        return params_;
    }
    virtual std::string get_json_params() const override;
private:

    virtual void on_tick(const tick_data& tick) override;

    virtual void on_bar(timeframe_type time_frame, const bar_data& bar) override;

    bool get_bars(timeframe_type tf, int many, bar_array_type& latest_bars) const;

    bool is_spread_within_limit(const tick_data& tick) const;

    bool check_orders_closing(const tick_data& tick);

    // check if cycle should be closed due to params.close_on_profit reached
    bool is_close_on_profit(const tick_data& tick);

    bool is_change_lots_allowed(double change_lots) const;

    void check_params() const;

    void try_opening_orders(const tick_data& tick);

    template <class T>
    bool order_send(const tick_data& tick, double volume, double open_price, int sl, int tp, custom_data_ptr data_ptr = nullptr)
    {
        try
        {
            order_ptr optr = std::make_shared<T>
                (engine_ptr_->get_symbol(), volume
                    , open_price, sl, tp);
            if (data_ptr)
            {
                optr->set_custom_data(data_ptr);
            }

            optr->open(tick);
            get_opened_orders().push_back(optr);

            get_event_queue().push_order_opened_event(optr);
        }
        catch (const std::exception&)
        {
            DEBUG_TRACE("Failed to create order");
            return false;
        }
        return true;
    }

    bool is_dynamic_change_allowed(const tick_data& tick, double lots_to_change) const;

    order_ptr find_profitable_buy(const tick_data& tick) const;

    order_ptr find_profitable_sell(const tick_data& tick) const;

    bool is_soft_tp_closed(const tick_data& tick);

    void set_info_xml(const tick_data& tick);

    bool plr_close(const tick_data& tick);

public:

    struct custom_data :public base_order::custom_data
    {
        custom_data(double near_lvl) :
            nearest_lvl(near_lvl),
            computed_profit(0)
        {}
        double nearest_lvl;
        double computed_profit;
    };

private:
    friend class ladder_strategy_compute;
    ladder_strategy_compute compute_;

    params params_;
    unsigned int total_trades_;

    double cycle_profit_;

    std::map<double, size_t> spread_counter_;
};

} // namespace fx
