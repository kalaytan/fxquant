#pragma once
#include <map>
#include <list>
#include <vector>
#include <atomic>
#include <thread>
#include <memory>
#include "event.h"
#include "order.h"
#include "socket.h"
#include "bar_data.h"
#include "tick_data.h"
#include "gui_context.h"

namespace fx {

class gui_server // singleton
{
public:
    typedef std::pair<order_cptr, order_action> order_info;

    struct options
    {
        int speed_factor_;
    };

public:
    static gui_server& instance()
    {
        static gui_server server_instance;
        return server_instance;
    }

    // delete copy and move constructors and assign operators
    gui_server(gui_server const&) = delete;
    gui_server(gui_server&&) = delete;
    gui_server& operator=(gui_server const&) = delete;
    gui_server& operator=(gui_server &&) = delete;

    bool on_tick(symbol sym, const tick_data& tick);
    bool on_bar(symbol sym, timeframe_type tf, const bar_data& bar);
    bool on_order(order_cptr optr, order_action action);
    bool on_info(symbol sym, const std::string& xml);

    void get_orders(symbol sym, std::vector<order_info>& orders);
    std::string get_info(symbol sym);

    bool get_options(symbol sym, options& opt) const;
    bool set_options(symbol sym, const options& opt);
    
private:
    gui_server();
    ~gui_server();

    void server_thread();
    void cleanup_thread();

    bool add_context(gui_context_ptr context_ptr);

    void on_abort()
    {
        abort_event_.signal();
    }

    void save_order(order_cptr optr, order_action action);
    void save_info(symbol sym, const std::string& info_xml);

private:
    network::tcp_socket_ptr sock_ptr_;
    
    std::atomic_bool shutdown_;
    std::unique_ptr<std::thread> server_thread_ptr_;
    std::unique_ptr<std::thread> cleanup_thread_ptr_;
    
    mutable std::mutex list_lock_;
    std::list<gui_context_ptr> context_list_;
    event abort_event_;

    mutable std::mutex orders_lock_;
    std::map<symbol, std::vector<order_info>> orders_;

    mutable std::mutex info_lock_;
    std::map<symbol, std::string> info_;

    mutable std::mutex options_lock_;
    std::map<symbol, options> options_;
};

} // namespace fx
