#include "ta.h"
#include "debug.h"
#include "fx_engine.h"
#include "engine_registry.h"
#include "default_strategy.h"
#include <json/json.h>

namespace fx {

void default_strategy::on_tick(const tick_data& tick)
{
    DEBUG_REQUIRE(!!engine_ptr_);

    // handle the 'ready to open' orders
    fx_engine::order_list opening_orders;

    // extract_opening_orders
    auto it = std::stable_partition(get_pending_orders().begin(), get_pending_orders().end(),
        [&tick](order_ptr optr) { return !optr->check_open(tick); });
    opening_orders.insert(opening_orders.end(), std::make_move_iterator(it),
        std::make_move_iterator(get_pending_orders().end()));
    get_pending_orders().erase(it, get_pending_orders().end());

    for (auto optr : opening_orders)
    {
        if (optr->open(tick)) // open the order
        {
            // move order to the list of opened orders
            get_opened_orders().push_back(optr);

            // inform the engine that the order was opened 
            get_event_queue().push_order_opened_event(optr);
        }
    }

    // handle the 'ready to close' orders
    fx_engine::order_list closing_orders;

    // extract_closing_orders
    it = std::stable_partition(get_opened_orders().begin(), get_opened_orders().end(),
        [&tick](order_ptr optr) { return !optr->check_close(tick); });
    closing_orders.insert(closing_orders.end(), std::make_move_iterator(it),
        std::make_move_iterator(get_opened_orders().end()));
    get_opened_orders().erase(it, get_opened_orders().end());

    for (auto optr : closing_orders)
    {
        if (optr->close(tick))
        {
            // move order to the list of closed orders
            get_closed_orders().push_back(optr);

            // inform the engine that the order was closed
            get_event_queue().push_order_closed_event(optr);
        }
    }

#if 0
    if (get_opened_orders().empty())
    {
        double sl = tick.get_bid() - 0.001;
        double tp = undefined_value<double>();

        try
        {
            order_ptr optr = std::make_shared<sell_order>
                (symbol::eurusd, 1000, tick.get_bid(), tp, sl);
            optr->open(tick);
            get_opened_orders().add(optr);
        }
        catch (const std::exception&)
        {
        }
    }
#endif
}

void default_strategy::on_bar(timeframe_type tf, const bar_data& bar)
{
    auto& bars = engine_ptr_->get_bar_collector();

    size_t size = bars.count(tf);
    DEBUG_TRACE("tf=%d bars=%lu", tf.count(), size);

    if (size >= params_.ma_period_)
    {
        data_array_type data;
        bars.get_bar_data(tf, params_.field_, size - params_.ma_period_, params_.ma_period_, data);

        data_array_type ma;
        calc_ma(data, params_.ma_period_, params_.ma_algo_, 1, ma);

#if 0
        auto eptr = engine_registry::instance().get_engine(symbol::eurusd);

        if (eptr)
        {
            auto& bc = eptr->get_bar_collector();
            auto count = bc.count(tf);
        }
#endif
    }
}

std::string default_strategy::get_json_params() const
{
    Json::Value root;
    root["ma_algo"] = static_cast<int>(params_.ma_algo_);
    root["ma_period"] = params_.ma_period_;

    Json::StyledWriter styledWriter;
    return styledWriter.write(root);
}

} // namespace fx
