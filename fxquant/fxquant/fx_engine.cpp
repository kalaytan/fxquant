#include "debug.h"
#include "strategy.h"
#include "fx_engine.h"
#include "gui_server.h"
#include <chrono>

namespace fx {

fx_engine::fx_engine(data_feeder_ptr feeder_ptr, strategy_ptr sptr,
    data_callback_ptr dcb_ptr, order_callback_ptr ocb_ptr) :
    feeder_ptr_(feeder_ptr), strategy_ptr_(sptr), last_order_id_(0),
    feeder_callback_ptr_(std::make_shared<feeder_callback>(*this)),
    data_events_(std::make_shared<data_event_callback>(*this, dcb_ptr)),
    order_events_(*this, ocb_ptr)
{
    DEBUG_REQUIRE(feeder_ptr_);
    DEBUG_REQUIRE(feeder_callback_ptr_);
    DEBUG_REQUIRE(strategy_ptr_);

    strategy_ptr_->set_engine(this);
    feeder_ptr_->add_callback(feeder_callback_ptr_);
}

fx_engine::~fx_engine()
{
    feeder_ptr_->remove_callback(feeder_callback_ptr_);
    DEBUG_TRACE("~fx_engine()");
}

bool fx_engine::submit_order(order_cptr optr, order_id_type& id)
{
    if (optr)
    {
        auto clone_ptr = optr->clone(); // create a copy of the input order
        //clone_ptr->set_id(++last_order_id_); // and assign an id to it
        id = clone_ptr->get_id(); // return id to the calling code

        // add order to the list of pending orders
        std::lock_guard<std::mutex> lock(lock_);
        new_orders_.push_back(clone_ptr);
        return true;
    }

    return false;
}

bool fx_engine::modify_order(order_id_type id, order_cptr optr)
{
    //// auto p = closed_orders_.find(id);
    //auto it = std::find_if(closed_orders_.begin(), closed_orders_.end(),
    //    [id](order_ptr optr) { return optr->get_id() == id; });

    //if (it != closed_orders_.end())
    //{
    //    auto p = *it;

    //    if (p && !p->is_closed())
    //    {
    //        if (optr->get_stop_loss() != p->get_stop_loss())
    //        {
    //            p->set_stop_loss(optr->get_stop_loss());
    //        }

    //        if (optr->get_take_profit() != p->get_take_profit())
    //        {
    //            p->set_take_profit(optr->get_take_profit());
    //        }

    //        if (optr->get_comment() != p->get_comment())
    //        {
    //            p->set_comment(optr->get_comment());
    //        }

    //        order_events_.push_order_modified_event(p);
    //        return true;
    //    }
    //}

    return false; // order not found
}

bool fx_engine::delete_order(order_id_type id)
{
    //// try to delete the closed order
    //order_ptr optr = nullptr;

    //auto it = std::find_if(closed_orders_.begin(), closed_orders_.end(),
    //    [&id](order_ptr optr) { return optr->get_id() == id; });    

    //if (it != closed_orders_.end()) // closed order not found
    //{
    //    optr = *it;
    //    closed_orders_.erase(it);
    //}
    //else
    //{
    //    // try to delete the pending order
    //    auto it = std::find_if(pending_orders_.begin(), pending_orders_.end(),
    //        [&id](order_ptr optr) { return optr->get_id() == id; });

    //    if (it != pending_orders_.end())
    //    {
    //        optr = *it;
    //        pending_orders_.erase(it);
    //    }
    //}

    //if (optr)
    //{
    //    // order deleted - inform the engine about this
    //    order_events_.push_order_deleted_event(optr);
    //    return true;
    //}

    return false; // order not found
}

void fx_engine::add_new_orders()
{
    std::lock_guard<std::mutex> lock(lock_);

    for (auto optr : new_orders_)
    {
        pending_orders_.push_back(optr);
        order_events_.push_order_submitted_event(optr);
    }
}

// fx_engine::data_event_callback

fx_engine::data_event_callback::data_event_callback(fx_engine& e, data_callback_ptr cb_ptr) :
    engine_(e), cb_ptr_(cb_ptr)
{
}

void fx_engine::data_event_callback::on_tick(const tick_data& tick)
{
    engine_.latest_tick_ = tick;
    engine_.add_new_orders();

    engine_.info_data_.reset();

    // push the tick into strategy instance
    engine_.strategy_ptr_->on_tick(tick);
    gui_server::instance().on_tick(engine_.get_symbol(), tick);

    if (!engine_.info_data_.is_empty())
    {
        auto xml_info_msg = engine_.info_data_.make_xml();
        gui_server::instance().on_info(engine_.get_symbol(), xml_info_msg);
    }

    if (cb_ptr_)
    {
        // call the external callback function
        cb_ptr_->on_tick(tick);
    }
}

void fx_engine::data_event_callback::on_bar(timeframe_type tf, const bar_data& bar)
{
    ALWAYS_TRACE("data_events_.size()=%lu", engine_.data_events_.size());

    // save this bar
    engine_.bars_.put_bar(tf, bar);

    // push the bar into strategy instance
    engine_.strategy_ptr_->on_bar(tf, bar);
    gui_server::instance().on_bar(engine_.get_symbol(), tf, bar);

    if (cb_ptr_)
    {
        // call the external callback function
        cb_ptr_->on_bar(tf, bar);
    }
}

void fx_engine::calc_open_trades_stats()
{
    auto& stats = strategy_ptr_->stats_;
    double& sum_profit = stats.opened_profit;
    int& wins = stats.opened_wins;
    int& loses = stats.opened_loses;

    // reset values
    sum_profit = 0;
    wins = 0;
    loses = 0;

    for (auto optr : opened_orders_)
    {
        double profit = optr->get_profit(get_latest_tick());

        if (profit >= 0)
        {
            wins++;
        }
        else
        {
            loses++;
        }

        sum_profit += profit;
    }
}

void fx_engine::calc_closed_trades_stats()
{
    const order_list& closed_orders = closed_orders_;

    double profit = 0;
    int wins = 0;
    int loses = 0;
    double lowest_profit = 0;
    double highest_profit = 0;

    int max_profits_in_row = 0;
    int current_profit_count = 0;
    bool is_last_profit = false;
    int max_loses_in_row = 0;
    int current_loss_count = 0;
    bool is_last_loss = false;

    timepoint_type open_time;
    timepoint_type closed_time;

    if (!closed_orders.empty())
    {
        auto first_element = *(closed_orders.begin());
        auto last_element = *(closed_orders.rbegin());
        open_time = first_element->get_open_tick().get_time();
        closed_time = last_element->get_close_tick().get_time();

        for (auto optr : closed_orders)
        {
            double pr = optr->get_profit();
            profit += pr;

            if (pr >= 0)
            {
                wins++;
                current_loss_count = 0;
                //current_profit_count++;

                if (++current_profit_count > max_profits_in_row)
                {
                    max_profits_in_row = current_profit_count;
                }
            }
            else
            {
                loses++;
                current_profit_count = 0;

                max_loses_in_row = ++current_loss_count > max_loses_in_row ? current_loss_count : max_loses_in_row;
                /*if (++current_loss_count > max_loses_in_row)
                {
                max_loses_in_row = current_loss_count;
                }*/
            }

            lowest_profit = profit < lowest_profit ? profit : lowest_profit;
            highest_profit = profit > highest_profit ? profit : highest_profit;
        }
    }

    size_t total_trades = closed_orders.size();
    auto& st = strategy_ptr_->stats_;

    st.total_closed_trades = total_trades;
    st.closed_wins = wins;
    st.closed_loses = loses;
    st.max_profits_in_row = max_profits_in_row;
    st.max_loses_in_row = max_loses_in_row;
    st.open_time = open_time;
    st.closed_time = closed_time;
    st.close_profit = profit;
    st.max_profit = highest_profit;
    st.min_profit = lowest_profit;
}


} // namespace fx
