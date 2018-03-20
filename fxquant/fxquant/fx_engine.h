#pragma once
#include <list>
#include <vector>
#include <memory>
#include "bar_data.h"
#include "info_data.h"
#include "tick_data.h"
#include "data_feeder.h"
#include "bar_collector.h"
#include "data_callback.h"
#include "order_callback.h"
#include "data_event_queue.h"
#include "order_event_queue.h"

namespace fx {

class strategy; // forward declaration
typedef std::shared_ptr<strategy> strategy_ptr;

class fx_engine
{
public:
    typedef std::list<order_ptr> order_list;

    fx_engine(
        data_feeder_ptr df_ptr,
        strategy_ptr sptr,
        data_callback_ptr dcb_ptr = nullptr,
        order_callback_ptr ocb_ptr = nullptr);

    ~fx_engine();

    // submit a new order
    bool submit_order(order_cptr optr, order_id_type& id);

    // modify the existing order
    bool modify_order(order_id_type id, order_cptr optr);

    // delete a closed or pending order
    bool delete_order(order_id_type id);

    const bar_collector& get_bar_collector() const
    {
        return bars_;
    }

    data_feeder_ptr get_feeder() const
    {
        return feeder_ptr_;
    }

    const order_list& get_closed_orders() const
    {
        return closed_orders_;
    }

    symbol get_symbol() const
    {
        return feeder_ptr_->get_symbol();
    }

    double get_point() const
    {
        return symbol_pip(get_symbol());
    }

    void calc_open_trades_stats();

    void calc_closed_trades_stats();

    const tick_data& get_latest_tick() const
    {
        return latest_tick_;
    }

private:
    friend class strategy;

    class data_event_callback : public data_callback
    {
    public:
        data_event_callback(fx_engine& e, data_callback_ptr cb_ptr);

    private:
        void on_tick(const tick_data& tick) override;
        void on_bar(timeframe_type tf, const bar_data& bar) override;

    private:
        fx_engine& engine_;
        data_callback_ptr cb_ptr_;
    };

    class feeder_callback : public data_callback
    {
    public:
        feeder_callback(fx_engine& e) : engine_(e) {}

    private:
        void on_tick(const tick_data& tick) override
        {
            engine_.data_events_.push_tick_event(tick);
        }

        void on_bar(timeframe_type time_frame, const bar_data& bar) override
        {
            engine_.data_events_.push_bar_event(time_frame, bar);
        }

    private:
        fx_engine& engine_;
    };

    void add_new_orders();

private:
    const data_feeder_ptr feeder_ptr_;
    const strategy_ptr strategy_ptr_;

    order_id_type last_order_id_;
    data_callback_ptr feeder_callback_ptr_;

    // queues of orders
    order_list new_orders_;
    order_list pending_orders_;
    order_list opened_orders_;
    order_list closed_orders_;

    bar_collector bars_; // NOTE: bars_ must be declared *before* data_events_!
    tick_data latest_tick_;

    data_event_queue data_events_;
    order_event_queue order_events_;

    info_data info_data_;

    mutable std::mutex lock_;
};

typedef std::shared_ptr<fx_engine> engine_ptr;
typedef std::shared_ptr<const fx_engine> engine_cptr;

} // namespace fx
