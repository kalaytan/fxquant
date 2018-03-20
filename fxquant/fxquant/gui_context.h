#pragma once
#include <map>
#include <mutex>
#include <memory>
#include <atomic>
#include <thread>
#include <vector>
#include <functional>
#include "event.h"
#include "order.h"
#include "symbol.h"
#include "socket.h"
#include "message.h"
#include "bar_data.h"
#include "tick_data.h"
#include "bar_collector.h"

namespace fx {

class gui_context
{
public:
    typedef std::function<void()> callback_func;

    gui_context(network::tcp_socket_ptr sock_ptr, callback_func on_abort);
    ~gui_context();

    bool start();

    bool add_bar(symbol sym, timeframe_type tf, const bar_data& bar);
    bool add_bars(symbol sym, timeframe_type tf, const std::vector<bar_data>& bars);
    bool add_tick(symbol sym, const tick_data& tick);
    bool add_order(order_cptr optr, order_action action);
    bool add_orders(symbol sym);
    bool add_info(symbol sym, const std::string& info_xml);

    bool is_aborted() const
    {
        return aborted_;
    }

private:
    bool initialize(symbol sym, const bar_collector& bc);

    bool send_bars();
    bool send_ticks();
    bool send_orders();
    bool send_info();
    bool send_status(status e);

    void send_thread_func();
    void recv_thread_func();

    bool receive_message(std::string& xml, timeout_type timeout);
    bool send_message(const std::string& xml);

    void set_aborted();

private:
    struct state
    {
        std::map<timeframe_type, std::vector<bar_data>> bars_;
        std::vector<tick_data> ticks_;
        std::vector<std::pair<order_cptr, order_action>> orders_;
        std::string info_;
    };

private:
    network::tcp_socket_ptr sock_ptr_;
    callback_func on_abort_;
    
    std::atomic_bool aborted_;
    std::atomic_bool canceled_;
    std::atomic_flag started_;

    mutable std::mutex data_lock_;
    mutable std::mutex send_lock_;

    mutable event data_event_;
    std::map<symbol, state> state_map_;
    
    std::unique_ptr<std::thread> send_thread_ptr_;
    std::unique_ptr<std::thread> recv_thread_ptr_;

    std::atomic<size_t> bar_count_;
};

typedef std::shared_ptr<gui_context> gui_context_ptr;

} // namespace fx
