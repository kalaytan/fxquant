#pragma once
#include <memory>
#include "types.h"
#include "bar_data.h"
#include "tick_data.h"
#include "order_event_queue.h"
#include "fx_engine.h"

namespace fx {

class strategy // a base class for all strategies
{
public:
    struct stats
    {
        // stats for closed trades
        double close_profit;
        size_t total_closed_trades;
        int closed_wins;
        int closed_loses;
        double max_profit;
        double min_profit;
        int max_profits_in_row;
        int max_loses_in_row;
        timepoint_type open_time;
        timepoint_type closed_time;

        // stats for open trades
        size_t total_opened_trades;
        int opened_wins;
        int opened_loses;
        double opened_profit;

        int get_total_opened_trades() const
        {
            return opened_wins + opened_loses;
        }

        double get_total_profit() const
        {
            return close_profit + opened_profit;
        }

        // TODO change operator to include open trades logic(profits)
        bool operator <(const stats& r) const
        {
            if (close_profit < r.close_profit)
            {
                return false;
            }
            else if (close_profit == r.close_profit)
            {
                return (total_closed_trades > r.total_closed_trades);
            }
            return true;
        }
    };

public:
    strategy() : engine_ptr_(nullptr) {}
    virtual ~strategy() = default;

    virtual bool print_params() const;
    virtual void print_simple_report() const;
    const stats& get_stats() const
    {
        return stats_;
    }
    virtual std::string get_csv_line() const;
    virtual std::string get_csv_header() const;

    double get_equity(const tick_data& tick) const;

    double normalize(double d) const
    {
        return engine_ptr_->feeder_ptr_->normalize(d);
    }

protected:
    fx_engine::order_list& get_pending_orders()
    {
        return engine_ptr_->pending_orders_;
    }

    fx_engine::order_list& get_opened_orders()
    {
        return engine_ptr_->opened_orders_;
    }

    const fx_engine::order_list& get_opened_orders() const
    {
        return engine_ptr_->opened_orders_;
    }

    fx_engine::order_list& get_closed_orders()
    {
        return engine_ptr_->closed_orders_;
    }

    const fx_engine::order_list& get_closed_orders() const
    {
        return engine_ptr_->closed_orders_;
    }

    order_event_queue& get_event_queue()
    {
        return engine_ptr_->order_events_;
    }

    bool close_trade(const tick_data& tick, order_ptr close_optr);

    void close_all_trades(const tick_data& tick);

    template <typename T>
    void add_xml_line(const std::string& key, const T& t)
    {
        engine_ptr_->info_data_.add_line(key, t);
    };

private:
    friend class fx_engine;
    virtual std::string get_json_params() const = 0;

    // called by fx_engine
    void set_engine(fx_engine* eptr)
    {
        engine_ptr_ = eptr;
        point_ = engine_ptr_->get_point();
    }

    // pure virtual functions
    virtual void on_tick(const tick_data& tick) = 0;
    virtual void on_bar(timeframe_type tf, const bar_data& bar) = 0;

protected:
    fx_engine* engine_ptr_;
    stats stats_;
    double point_;
}; // class strategy

bool operator <(strategy_ptr a, strategy_ptr b);

} // namespace fx
