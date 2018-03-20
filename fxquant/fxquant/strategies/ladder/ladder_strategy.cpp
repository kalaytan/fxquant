#include <cmath>
#include <sstream>
#include <algorithm>
#include <json/json.h>

#include "ta.h"
#include "utils.h"
#include "debug.h"
#include "config.h"
#include "common.h"
#include "logger.h"
#include "fx_engine.h"
#include "engine_registry.h"
#include "ladder_strategy.h"
#include "gui_server.h"

using namespace std::placeholders;
using namespace std::chrono_literals;
using namespace std::chrono;

namespace fx {

ALWAYS_INLINE bool on_level(order_ptr optr, double lvl)
{
    ladder_strategy::custom_data& cd = static_cast<ladder_strategy::custom_data&>(*(optr->get_custom_data()));
    return ((int)(cd.nearest_lvl * 10000) == (int)(lvl * 10000));
}

bool compare_profits(const order_ptr& a, const order_ptr& b)
{
    auto aptr = a->get_custom_data();
    auto bptr = b->get_custom_data();

    if (aptr && bptr)
    {
        auto a_cd = static_cast<ladder_strategy::custom_data&>(*aptr);
        auto b_cd = static_cast<ladder_strategy::custom_data&>(*bptr);

        return a_cd.computed_profit < b_cd.computed_profit;
    }

    if (!aptr && bptr)
    {
        return true;
    }

    return false;
}

std::string time_point_to_string(std::chrono::system_clock::time_point &tp)
{
    auto ttime_t = system_clock::to_time_t(tp);
    auto tp_sec = system_clock::from_time_t(ttime_t);
    milliseconds ms = duration_cast<milliseconds>(tp - tp_sec);

    std::tm * ttm = gmtime(&ttime_t);
    char date_time_format[] = "%Y-%m-%d %H:%M:%S";
    char time_str[256] = { 0 };
    strftime(time_str, sizeof(time_str), date_time_format, ttm);

    std::string result(time_str);
    result.append(".");
    result.append(std::to_string(ms.count()));

    return result;
}

ladder_strategy::ladder_strategy() :
    compute_(*this),
    total_trades_(0),
    cycle_profit_(0)
{
    params_.volume = config::get_double("params", "volume");
    params_.sl = config::get_int("params", "sl");
    params_.tp = config::get_int("params", "tp");
    params_.soft_tp = config::get_int("params", "soft_tp");
    params_.step = config::get_int("params", "step");
    params_.lvl_tolerance = config::get_int("params", "lvl_tolerance");
    params_.trades_per_lvl = config::get_int("params", "trades_per_lvl");
    params_.close_cycle_on_profit = config::get_double("params", "close_cycle_on_profit");
    params_.sections_offset = config::get_int("params", "sections_offset");
    params_.sections_in_half_range = config::get_int("params", "sections_in_half_range");
    params_.max_lots_allowed = config::get_double("params", "max_lots_allowed");
    params_.max_spread = config::get_int("params", "max_spread");
    params_.plr = config::get_int("params", "plr");
    check_params();

    for (double d = 0.00005; d <= 0.0005; d += 0.00003)
    {
        spread_counter_.insert({ d, 0 });
    }
    spread_counter_.insert({ std::numeric_limits<double>::infinity(), 0 });
}

ladder_strategy::ladder_strategy(const params& par) :
    compute_(*this),
    params_(par)
    , total_trades_(0)
    , cycle_profit_(0)
{
    check_params();
}

void ladder_strategy::check_params() const
{
    if (params_.volume < 0.01
        || params_.sl < 0
        || params_.tp < 0
        || params_.soft_tp < 0
        || params_.step <= 0
        || params_.lvl_tolerance <= 0
        || params_.trades_per_lvl <= 0
        || params_.close_cycle_on_profit < 0
        || params_.sections_in_half_range < 0
        || params_.max_lots_allowed < 0
        || params_.max_spread < 0
        )
    {
        throw std::invalid_argument("Incoming invalid param ");
    }
}

ladder_strategy::~ladder_strategy()
{
    //DEBUG_TRACE("---------------- spread: %i , spread above: %i", count1, count2);
}

bool ladder_strategy::is_dynamic_change_allowed(const tick_data& tick, const double lots_to_change) const
{
    if (params_.max_lots_allowed <= 0)
    {
        return true;
    }

    double range = 0;
    int section = compute_.get_section();
    double allowed_risk = fabs(compute_.get_dynamic_section_risk());
    double current_lots = compute_.get_lots_exposure();
    double new_lots = current_lots + lots_to_change;

    if (section > 0)
    {
        new_lots = fabs(new_lots);
        if (new_lots <= allowed_risk) // new lots withing a limit
        {
            return true;
        }
        else
        {
            // return true if new lots changing towards limit but not away from it.
            return  (new_lots < fabs(compute_.get_lots_exposure()));
        }
    }
    else
    {
        if (compute_.is_top_half())
        {
            if (new_lots >= allowed_risk) // new lots above allowed
            {
                return true;
            }

            if (new_lots >= current_lots) // changing towards allowed
            {
                return true;
            }
        }
        else
        {
            if (new_lots <= -allowed_risk) // new lots below allowed
            {
                return true;
            }

            if (new_lots <= current_lots) // lots are changing towards allowed
            {
                return true;
            }
        }
    }
    return false;

}

void ladder_strategy::print_spread() const
{
#if !defined(NDEBUG) // debug build
    for (const auto& pair : spread_counter_)
    {
        DEBUG_TRACE("spread: %f -> %lu", pair.first, pair.second);
    }
#endif
}

bool ladder_strategy::is_spread_within_limit(const tick_data& tick) const
{
    if (params_.max_spread <= 0)
    {
        return true;
    }

    return tick.get_spread() <= params_.max_spread;
}

void ladder_strategy::on_tick(const tick_data& tick)
{
    compute_.initialize(tick);
    set_info_xml(tick);

    //#if !defined(NDEBUG) // debug build
    //    double spread = tick.get_ask() - tick.get_bid();
    //    for (auto& pair : spread_counter_)
    //    {
    //        if (spread <= pair.first)
    //        {
    //            pair.second++;
    //            break;
    //        }
    //    }
    //#endif

    try_opening_orders(tick);

    check_orders_closing(tick);

    if (is_close_on_profit(tick))
    {
        close_all_trades(tick);
        // TODO send flag to gui
        logger::instance().info("Cycle_Profit: " + std::to_string(cycle_profit_)
            + ". Balance: " + std::to_string(compute_.get_close_profit() + compute_.get_equity()));
        //DEBUG_TRACE("Cycle_Profit: %f. Balance: %f", cycle_profit_, compute_.get_close_profit() + compute_.get_equity());
        cycle_profit_ = 0;
    }


    is_soft_tp_closed(tick);
}

/*
Generates xml string simmilar to below data

EURUSD - 2 Feb 2016 - 12 Feb 2016
spread = 12
profit = 123
buys = lots, # of trades
sells = lots, # of trades
lots risk(total buys - total sells)
*/
void ladder_strategy::set_info_xml(const tick_data& tick)
{
    add_xml_line("symbol", symbol_to_string(engine_ptr_->get_symbol()));
    add_xml_line("tick_time", time_point_to_string(tick.get_time()));
    add_xml_line("spread", tick.get_spread());
    add_xml_line("section_iam_in", compute_.get_section());
    add_xml_line("close_profit", compute_.get_close_profit());
    add_xml_line("equity", compute_.get_equity());
    add_xml_line("trades_buys", compute_.get_open_buy_trades());
    add_xml_line("trades_sells", compute_.get_open_sell_trades());
    add_xml_line("volume_buys", compute_.get_open_buy_volume());
    add_xml_line("volume_sells", compute_.get_open_sell_volume());
    add_xml_line("hedge", compute_.get_lots_exposure());
}

// PLR = Profitable Lots Reducer
// 1. take biggest winer
// 2. find and close bigest loser where:
// - oposite type (buy & sell)
// - with profit > CONFIG_PLR
bool ladder_strategy::plr_close(const tick_data& tick)
{
    if (get_opened_orders().empty())
    {
        return false;
    }

    get_opened_orders().sort(compare_profits);

    // get biggest winner
    auto winner = *(get_opened_orders().rbegin());
    DEBUG_ASSERT(winner->get_custom_data());
    auto cd_winner = static_cast<ladder_strategy::custom_data&>(*(winner->get_custom_data()));

    if (cd_winner.computed_profit <= params_.plr)
    {
        return false;
    }

    bool closed_trades = false;
    double close_profit = 0;
    // calc max acceptable loss
    //double max_allowed_loss = -(cd_winner.computed_profit - CONFIG_PLR);
    //double min_allowed_loss = ()
    //DEBUG_ASSERT(max_allowed_loss < 0);

    for (auto optr : get_opened_orders())
    {
        DEBUG_ASSERT(optr->get_custom_data());
        auto cd = static_cast<ladder_strategy::custom_data&>(*(optr->get_custom_data()));

        // go to next trade if loss is to big
        //     0.00035                                     <   0.00050 = true

        //                                       0.000215  <   0.00050 = false
        //      0.00250               +   -0.00035         <   0.00050 
        if (cd_winner.computed_profit + cd.computed_profit < params_.plr * point_)
        {
            continue;
        }

        // exit the loop if losing trade is positive.
        if (cd.computed_profit > 0)
        {
            break;
        }

        // skip if trades of same type (same direction)
        if (derived_from<buy_order>(winner) == derived_from<buy_order>(optr))
        {
            continue;
        }

        // close both trades
        if (close_trade(tick, optr))
        {
            if (close_trade(tick, winner))
            {
                closed_trades = true;
                close_profit = winner->get_profit() - optr->get_profit();
                DEBUG_TRACE("Closed 2 trades in PLR. profit: %f", close_profit);
                break;
            }
            else
            {
                // error PLR first trade closed but second failed
                DEBUG_TRACE("ERROR: failed to close winning trade in PLR.");
                break;
            }
        }
    }

    if (closed_trades)
    {
        DEBUG_ENSURE(close_profit >= params_.plr);
        return true;
    }
    return false;
}

bool ladder_strategy::is_soft_tp_closed(const tick_data& tick)
{
    if (params_.soft_tp <= 0 || get_opened_orders().empty()) // quit if disabled in params
    {
        return false;
    }

    bool is_closed = false;
    order_ptr smallest_profitable_buy = find_profitable_buy(tick);

    if (smallest_profitable_buy)
    {
        double change_lots = -smallest_profitable_buy->get_volume();
        if (is_dynamic_change_allowed(tick, change_lots))
        {
            if (close_trade(tick, smallest_profitable_buy))
            {
                cycle_profit_ += smallest_profitable_buy->get_profit();
                is_closed = true;
            }
        }
    }

    order_ptr smallest_profitable_sell = find_profitable_sell(tick);

    if (smallest_profitable_sell)
    {
        double change_lots = smallest_profitable_sell->get_volume();
        if (is_dynamic_change_allowed(tick, change_lots))
        {
            if (close_trade(tick, smallest_profitable_sell))
            {
                cycle_profit_ += smallest_profitable_sell->get_profit();
                is_closed = true;
            }
        }
    }

    return is_closed;
}

order_ptr ladder_strategy::find_profitable_buy(const tick_data& tick) const
{
    DEBUG_REQUIRE(point_ > 0);
    DEBUG_REQUIRE(tick.get_bid() > 0);
    DEBUG_REQUIRE(params_.soft_tp > 0);

    double profitable_price = tick.get_bid() - params_.soft_tp * point_;
    order_ptr result_ptr = nullptr;
    double profit = -std::numeric_limits<double>::infinity();
    // open_price <=  profitable_price
    for (auto optr : get_opened_orders())
    {
        if (
            derived_from<buy_order>(optr)
            && optr->get_open_price() <= profitable_price
            )
        {
            double cur_profit = optr->get_profit(tick);
            if (profit < cur_profit)
            {
                result_ptr = optr;
                profit = cur_profit;
                //DEBUG_TRACE("B: checking: %f", cur_profit);
            }
        }
    }
    /*if (result_ptr)
        DEBUG_TRACE("B: choosen profit: %f", result_ptr->get_profit(tick));*/
    return result_ptr;
}

order_ptr ladder_strategy::find_profitable_sell(const tick_data& tick) const
{
    DEBUG_REQUIRE(point_ > 0);
    DEBUG_REQUIRE(tick.get_ask() > 0);
    DEBUG_REQUIRE(params_.soft_tp > 0);

    double profitable_price = tick.get_ask() + params_.soft_tp * point_;
    order_ptr result_ptr = nullptr;
    double profit = -std::numeric_limits<double>::infinity();
    // open_price <=  profitable_price
    for (auto optr : get_opened_orders())
    {
        if (
            derived_from<sell_order>(optr)
            && optr->get_open_price() >= profitable_price
            )
        {
            double cur_profit = optr->get_open_price() - tick.get_ask();
            if (profit < cur_profit)
            {
                result_ptr = optr;
                profit = cur_profit;
                //DEBUG_TRACE("S: checking: %f", cur_profit);
            }
        }
    }
    /*if (result_ptr)
        DEBUG_TRACE("S: choosen profit: %f", result_ptr->get_profit(tick));*/
    return result_ptr;
}

bool ladder_strategy::is_close_on_profit(const tick_data& tick)
{
    if (params_.close_cycle_on_profit == 0)
    {
        return false;
    }
    add_xml_line("cycle_profit", cycle_profit_);
    // todo  - get_equity(t) calculated again? possibly should use compute_ version.
    // delete get_equity(t) ?
    double profit = cycle_profit_ + get_equity(tick);
    return (params_.close_cycle_on_profit > 0)
        && (profit >= params_.close_cycle_on_profit);
}

void ladder_strategy::try_opening_orders(const tick_data& tick)
{
    if (!is_spread_within_limit(tick))
    {
        return;
    }

    double nearest_level = compute_.get_nearest_level();

    // mid price touching the line?
    if (fabs(tick.get_mid_price() - nearest_level) <= params_.lvl_tolerance*point_)
    {
        // container for all trades on level
        fx_engine::order_list list;

        std::copy_if(get_opened_orders().begin(), get_opened_orders().end(), std::back_inserter(list),
            [nearest_level](order_ptr optr) { return on_level(optr, nearest_level); });

        int buys = 0;
        int sells = 0;

        if (!list.empty())
        {
            for (auto p : list)
            {
                // count buys on level
                if (derived_from<buy_order>(p))
                {
                    buys++;
                }
                // count sells on level
                else
                {
                    sells++;
                }
            }
        }

        if (
            buys < params_.trades_per_lvl
            && is_change_lots_allowed(params_.volume)
            )
        {
            auto cd_ptr = std::make_shared<custom_data>(nearest_level);
            order_send<buy_order>(tick, params_.volume, tick.get_ask(), params_.sl, params_.tp, cd_ptr);
        }

        if (
            sells < params_.trades_per_lvl
            && is_change_lots_allowed(-params_.volume)
            )
        {
            auto cd_ptr = std::make_shared<custom_data>(nearest_level);
            order_send<sell_order>(tick, params_.volume, tick.get_bid(), params_.sl, params_.tp, cd_ptr);
        }
    }
}

bool ladder_strategy::is_change_lots_allowed(double change_lots) const
{
    DEBUG_REQUIRE(change_lots != 0);

    if (params_.max_lots_allowed <= 0)
    {
        return true;
    }

    const double current_lots = compute_.get_lots_exposure();
    const double new_lots = current_lots + change_lots;
    const double new_lots_abs = fabs(new_lots);
    const double allowed_lots = params_.max_lots_allowed;

    if (compute_.get_section() >= 0)
    {
        if (new_lots_abs <= allowed_lots) // new lots is withing the limit
        {
            return true;
        }

        if (new_lots_abs < fabs(current_lots)) // new lots is moving towards the limit
        {
            return true;
        }
    }
    else // current section is < 0
    {
        if (compute_.is_top_half())
        {
            if (new_lots <= allowed_lots) // new lots is within the limit
            {
                return true;
            }

            if (new_lots > current_lots) // new lots is moving towards the limit
            {
                return true;
            }
        }
        else // bottom half of range
        {
            if (new_lots <= -allowed_lots) // new lots is within the limit
            {
                return true;
            }

            if (new_lots < current_lots)
            {
                return true;
            }
        }
    }

    return false;
}

// check sl & tp closing for all open orders
bool ladder_strategy::check_orders_closing(const tick_data& tick)
{
    if (get_opened_orders().empty())
    {
        return false;
    }
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
            cycle_profit_ += optr->get_profit();
            // move order to the list of closed orders
            get_closed_orders().push_back(optr);

            // inform the engine that the order was closed
            get_event_queue().push_order_closed_event(optr);
            return true;
        }
    }
    return false;
}

void ladder_strategy::on_bar(timeframe_type tf, const bar_data& bar)
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

    //char time_str[20];
    //strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", gmtime(&latest_bars[0].t));

    //DEBUG_TRACE("%s: legal lines. Buy line: %f. Sell line: %f.", time_str, buy_line_, sell_line_);
}

bool ladder_strategy::get_bars(timeframe_type tf, int many, bar_array_type& latest_bars) const
{
    auto& bars = engine_ptr_->get_bar_collector();

    bars.get_last_bars(tf, many, latest_bars);
    if (latest_bars.empty())
    {
        return false;
    }

    return true;
}

// prepare params to be printed in report
std::string ladder_strategy::get_json_params() const
{
    Json::Value root;
    root["volume"] = params_.volume;
    root["sl"] = params_.sl;
    root["tp"] = params_.tp;

    Json::StyledWriter styledWriter;
    return styledWriter.write(root);
}


//----------------  COMPUTE CLASS
void ladder_strategy_compute::set_open_trades_minmax_and_range(const tick_data& tick)
{
    const auto& list = strategy_.get_opened_orders();
    if (list.empty())
    {
        range_min_ = tick.get_bid();
        range_max_ = tick.get_bid();
        range_ = strategy_.point_;
    }
    else
    {
        auto minmax = std::minmax_element(list.begin(), list.end(),
            [](order_ptr a, order_ptr b) {return a->get_open_price() < b->get_open_price(); });

        range_min_ = (*minmax.first)->get_open_price();
        range_max_ = (*minmax.second)->get_open_price();

        if (range_max_ < tick.get_bid())
        {
            range_max_ = tick.get_bid();
        }

        if (range_min_ > tick.get_bid())
        {
            range_min_ = tick.get_bid();
        }

        range_ = strategy_.normalize(range_max_ - range_min_);
    }

    DEBUG_ENSURE(range_min_ > 0);
    DEBUG_ENSURE(range_max_ > 0);
    DEBUG_ENSURE(range_ > 0);
}

void  ladder_strategy_compute::set_range(const tick_data& tick)
{
    range_ = strategy_.normalize(get_range_max() - get_range_min());
    DEBUG_ENSURE(range_ > 0);
}

void ladder_strategy_compute::set_top_half(const tick_data& tick)
{
    DEBUG_REQUIRE(range_max_ != undefined_value<double>() && range_max_ > 0);
    DEBUG_REQUIRE(range_min_ != undefined_value<double>() && range_min_ > 0);
    DEBUG_REQUIRE(mid_price_ != undefined_value<double>() && mid_price_ > 0);

    // TODO doesn't check if no open trades
    double distance_to_top = range_max_ - mid_price_;
    double distance_to_bottom = mid_price_ - range_min_;

    is_top_half_ = (distance_to_top < distance_to_bottom);
}

void ladder_strategy_compute::set_section_i_am_in(const tick_data& tick)
{
    DEBUG_REQUIRE(range_max_ != 0);
    DEBUG_REQUIRE(range_min_ != 0);
    DEBUG_REQUIRE(range_ != 0);
    DEBUG_REQUIRE(mid_price_ != 0);

    double distance =
        strategy_.normalize
        //fixed_point<5>
        (
            range_max_ - mid_price_ > mid_price_ - range_min_ ?
            mid_price_ - range_min_ : range_max_ - mid_price_
        );
    //DEBUG_ENSURE(distance > 0);
    //            0.000015678 / 0.00001 = 1.567890 * 0.00001 = 0.00001
    //            0.00001     / 0.00001 = 1
    //                   0.00000156               100                             /     80
    section_ = (int)floor(distance * strategy_.params_.sections_in_half_range * 2 / range_)
        + strategy_.params_.sections_offset;
    //DEBUG_TRACE("SECTION: %d (distance=%f)", section_, distance);

    if (section_ >= -19)
    {
        int z = 1;
    }
}

void ladder_strategy_compute::set_dynamic_section_risk(const tick_data& tick)
{
    if (strategy_.params_.sections_in_half_range <= 0)
    {
        dynamic_section_risk_ = strategy_.params_.max_lots_allowed;
    }
    //  5.0            /           100             *  15 = 
    dynamic_section_risk_ = strategy_.params_.max_lots_allowed / strategy_.params_.sections_in_half_range * section_;
}

void ladder_strategy_compute::set_nearest_level(const tick_data& tick)
{
    DEBUG_REQUIRE(mid_price_ > 0);
    DEBUG_REQUIRE(strategy_.point_ > 0);
    DEBUG_REQUIRE(strategy_.params_.step > 0);
    int distance = static_cast<int>(mid_price_ / strategy_.point_) % strategy_.params_.step;
    if (distance > strategy_.params_.step / 2)
    {
        nearest_level_ = mid_price_ - (distance + strategy_.params_.step) * strategy_.point_;
    }
    nearest_level_ = (mid_price_ - distance*strategy_.point_);
    DEBUG_ENSURE(nearest_level_ > 0);
}

void ladder_strategy_compute::loop_opened_orders()
{
    lots_exposure_ = 0;

    for (auto optr : strategy_.get_opened_orders())
    {
        if (derived_from<buy_order>(optr))
        {
            open_buy_trades_++;
            open_buy_volume_ += optr->get_volume();
        }
        else
        {
            open_sell_trades_++;
            open_sell_volume_ += optr->get_volume();
        }
    }

    lots_exposure_ = open_buy_volume_ - open_sell_volume_;
}

void ladder_strategy_compute::loop_closed_orders()
{
    for (auto order_ptr : strategy_.get_closed_orders())
    {
        close_profit_ += order_ptr->get_profit();
    }
}

ladder_strategy_compute::ladder_strategy_compute(const ladder_strategy& strategy) :
    strategy_(strategy)
{
}

void ladder_strategy_compute::reset_values(const tick_data& tick)
{
    range_min_ = undefined_value<double>();
    range_max_ = undefined_value<double>();
    range_ = undefined_value<double>();
    is_top_half_ = false;
    mid_price_ = tick.get_mid_price();
    section_ = undefined_value<int>();
    dynamic_section_risk_ = undefined_value<double>();
    equity_ = 0;
    close_profit_ = 0;
    open_buy_trades_ = 0;
    open_sell_trades_ = 0;
    open_buy_volume_ = 0;
    open_sell_volume_ = 0;

    DEBUG_ENSURE(mid_price_ > 0 && mid_price_ >= tick.get_bid() && mid_price_ <= tick.get_ask());
}

void ladder_strategy_compute::initialize(const tick_data& tick)
{
    reset_values(tick);
    set_open_trades_minmax_and_range(tick);
    set_top_half(tick);
    set_section_i_am_in(tick);
    set_dynamic_section_risk(tick);
    set_nearest_level(tick);
    loop_opened_orders();
    set_profits(tick);
    loop_closed_orders();
}

void ladder_strategy_compute::set_profits(const tick_data& tick)
{
    for (auto optr : strategy_.get_opened_orders())
    {
        auto cd_ptr = optr->get_custom_data();
        if (cd_ptr)
        {
            ladder_strategy::custom_data& cd = static_cast<ladder_strategy::custom_data&>(*cd_ptr);
            cd.computed_profit = optr->get_profit(tick);
            equity_ += cd.computed_profit;
        }
    }
}

} // namespace fx
