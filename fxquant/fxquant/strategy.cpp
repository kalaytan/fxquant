#include "strategy.h"
#include "logger.h"
#include "utils.h"
#include "debug.h"

#include <json/json.h>
#include <json/reader.h>

namespace fx {
bool strategy::print_params() const
{
    std::string json = get_json_params();

    Json::Value root;
    Json::Reader reader;
    bool parsing_successful = reader.parse(json.c_str(), root);
    if (!parsing_successful)
    {
        return parsing_successful;
    }
    std::string msg;
    for (Json::ValueIterator it = root.begin(); it != root.end(); it++)
    {
        if (it->isConvertibleTo(Json::ValueType::stringValue))
        {
            msg += it.key().asString() + "=" + it->asString() + "; ";
        }
    }
    logger::instance().info(msg);
    return true;
}

void strategy::print_simple_report() const
{
    const auto& st = stats_;
    using namespace std;
    logger::instance().info("Total trades:" + to_string(st.total_closed_trades)
        + ". Wins:" + to_string(st.closed_wins) + ". Losses:" + to_string(st.closed_loses));
    logger::instance().info("Profits in row:" + to_string(st.max_profits_in_row)
        + ". Loses in row:" + to_string(st.max_loses_in_row));
    logger::instance().info(std::string("First trade open:") + time_to_string(st.open_time));
    logger::instance().info(std::string("Last trade close:") + time_to_string(st.closed_time));
    logger::instance().info("Profit: " + to_string(st.close_profit)
        + ". Highest profit:" + to_string(st.max_profit) + ". Lowest profit:" + to_string(st.min_profit));

    logger::instance().info("Open Trades:" + to_string(st.get_total_opened_trades()));
    logger::instance().info("Open Wins:" + to_string(st.opened_wins)
        + std::string(". Open Loses:") + to_string(st.opened_loses));
    logger::instance().info("Open trades profit: " + to_string(st.opened_profit));

    logger::instance().info("--- TOTAL PROFIT:" + to_string(st.get_total_profit()));
}

double strategy::get_equity(const tick_data& tick) const
{
    const auto& opened_orders = engine_ptr_->opened_orders_;
    double profit = 0;
    //DEBUG_TRACE("---------------------------");
    for (auto order : opened_orders)
    {
        double temp_profit = order->get_profit(tick);
        profit += temp_profit;
        //DEBUG_TRACE("profit: %f, total profit: %f", temp_profit, profit);
    }

    return profit;
}

std::string strategy::get_csv_line() const
{
    std::string line;

    // adding params to the line
    std::string json = get_json_params();
    Json::Value root;
    Json::Reader reader;
    bool parsing_successful = reader.parse(json.c_str(), root);

    if (parsing_successful)
    {
        for (Json::ValueIterator it = root.begin(); it != root.end(); it++)
        {
            if (it->isConvertibleTo(Json::ValueType::stringValue))
            {
                line += it->asString() + ";";
            }
        }
    }

    // adding results to the line
    const auto& st = stats_;
    line += std::to_string(st.total_closed_trades) + ";";
    line += std::to_string(st.closed_wins) + ";";
    line += std::to_string(st.closed_loses) + ";";
    line += std::to_string(st.max_profits_in_row) + ";";
    line += std::to_string(st.max_loses_in_row) + ";";
    line += time_to_string(st.open_time) + ";";
    line += time_to_string(st.closed_time) + ";";
    line += std::to_string(st.close_profit) + ";";
    line += std::to_string(st.max_profit) + ";";
    line += std::to_string(st.min_profit);

    return line;
}

std::string strategy::get_csv_header() const
{
    std::string line;

    // adding params to the line
    std::string json = get_json_params();
    Json::Value root;
    Json::Reader reader;
    bool parsing_successful = reader.parse(json.c_str(), root);

    if (parsing_successful)
    {
        for (Json::ValueIterator it = root.begin(); it != root.end(); it++)
        {
            if (it->isConvertibleTo(Json::ValueType::stringValue))
            {
                line += it.key().asString() + ";";
            }
        }
    }

    // adding results to the line
    const auto& st = stats_;
    line += "total_trades;";
    line += "wins;";
    line += "loses;";
    line += "max_profits_in_row;";
    line += "max_loses_in_row;";
    line += "open_time;";
    line += "closed_time;";
    line += "profit;";
    line += "highest_profit;";
    line += "lowest_profit";

    return line;
}

bool operator <(strategy_ptr a, strategy_ptr b)
{
    const auto& as = a->get_stats();
    const auto& bs = b->get_stats();

    if (as.close_profit < bs.close_profit)
    {
        return true;
    }

    if (as.close_profit > bs.close_profit)
    {
        return false;
    }

    if (as.max_profit < bs.max_profit)
    {
        return true;
    }

    if (as.max_profit > bs.max_profit)
    {
        return false;
    }

    return (a.get() < b.get());
}

bool strategy::close_trade(const tick_data& tick, order_ptr close_optr)
{
    auto it = get_opened_orders().begin();
    while (it != get_opened_orders().end())
    {
        auto optr = *it;
        if (optr == close_optr)
        {
            if (optr->close(tick))
            {
                // move order to the list of closed orders
                get_closed_orders().push_back(optr);

                // remove from open orders
                get_opened_orders().erase(it);

                // inform the engine that the order was closed
                get_event_queue().push_order_closed_event(optr);

                return true;
            }
        }
        ++it;
    }
    return false;
}

void strategy::close_all_trades(const tick_data& tick)
{
    for (auto optr : get_opened_orders())
    {
        if (optr->close(tick))
        {
            // move order to the list of closed orders
            get_closed_orders().push_back(optr);

            // inform the engine that the order was closed
            get_event_queue().push_order_closed_event(optr);
        }
    }
    get_opened_orders().clear();
}

}//namespace fx