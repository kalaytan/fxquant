#include <sstream>
#include <json/json.h>

#include "ta.h"
#include "debug.h"
#include "logger.h"
#include "fx_engine.h"
#include "engine_registry.h"
#include "first_strategy.h"

using namespace std::chrono_literals;

namespace fx {

void first_strategy::on_tick(const tick_data& tick)
{
    //// count total number of open trades
    //total_trades_ = get_total_opened_orders();

    //// get open price of latest candle
    //double level = get_level();
    //
    //if(level <= 0.0)
    //{
    //	return;
    //}
    if (buy_line_ <= 0 || sell_line_ <= 0)
    {
        return;
    }
    if (!get_opened_orders().empty())
    {
        check_trades(tick);
        return;
    }
    if (trade_opens_in_current_bar_ >= params_.max_trades_per_candle)
    {
        return;
    }

    // (buy_line_ - sell_line_) / 2  normolized to integer.
    int sl = params_.lines_dist / 2;
    int tp = sl;
    bool order_send_success = false;
    // buy
    if (tick.get_bid() >= buy_line_)
    {
        order_send_success = order_send<buy_order>(tick, tick.get_ask(), sl, tp);
    }
    // sell
    else if (tick.get_bid() <= sell_line_)
    {
        order_send_success = order_send<sell_order>(tick, tick.get_bid(), sl, tp);
    }

    if (order_send_success)
    {
        trade_opens_in_current_bar_++;
    }
    // logika zakrytiya
}
bool first_strategy::check_trades(const tick_data& tick)
{
    check_orders_closing(tick);
    return false;
}
bool first_strategy::check_orders_closing(const tick_data& tick)
{
    // handle the 'ready to close' orders
    fx_engine::order_list closing_orders;

    // extract_closing_orders
    auto it = std::stable_partition(get_opened_orders().begin(), get_opened_orders().end(),
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
            return true;
        }
    }
    return false;
}
void first_strategy::on_bar(timeframe_type tf, const bar_data& bar)
{
    // const timeframe_type use_tf = 1min ;
    if (tf != 1min)
    {
        return;
    }
    // get latest bars
    bar_array_type latest_bars;
    if (!get_bars(1min, 2, latest_bars))
    {
        return;
    }

    // exit if last candle smaller than allowed?
    if (is_bar_legal(-1, latest_bars))
    {
        buy_line_ = 0;
        sell_line_ = 0;
        return;
    }

    buy_line_ = latest_bars[0].o + params_.lines_dist * engine_ptr_->get_point() / 2;
    sell_line_ = buy_line_ - params_.lines_dist * engine_ptr_->get_point();

    // reset trades counter
    trade_opens_in_current_bar_ = 0;


    char time_str[20];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", gmtime(&latest_bars[0].t));

    //DEBUG_TRACE("%s: legal lines. Buy line: %f. Sell line: %f.", time_str, buy_line_, sell_line_);
}
bool first_strategy::get_bars(timeframe_type tf, int many, bar_array_type& latest_bars) const
{
    auto& bars = engine_ptr_->get_bar_collector();

    bars.get_last_bars(tf, many, latest_bars);
    if (latest_bars.empty())
    {
        return false;
    }

    return true;
}
//
// checks if the candle is valid or not according to strategy rules.
// 
bool  first_strategy::is_bar_legal(int bar, const bar_array_type& latest_bars) const
{
    size_t index = static_cast<size_t>(bar);
    if (bar < 0)
    {
        index = latest_bars.size() + bar;
    }
    if (index > latest_bars.size())
    {
        throw std::invalid_argument("Index is out of range.");
    }

    double point = engine_ptr_->get_point();
    double candle_size = (latest_bars[index].h - latest_bars[index].l) / point;

    if (candle_size <= params_.min_candle)
    {
        return false;
    }
    return true;
}

std::string first_strategy::get_json_params() const
{
    Json::Value root;
    root["volume"] = params_.volume;
    root["sl"] = params_.sl;
    root["tp"] = params_.tp;
    root["min_candle"] = params_.min_candle;
    root["lines_dist"] = params_.lines_dist;
    root["max_trades_per_candle"] = params_.max_trades_per_candle;

    Json::StyledWriter styledWriter;
    return styledWriter.write(root);
}

} // namespace fx
