#include "debug.h"
#include "logger.h"
#include "config.h"
#include "gui_server.h"

using namespace fx::network;
using namespace std::chrono_literals;

namespace fx {

gui_server::gui_server() : shutdown_(false)
{
    auto port = config::get_server_port();

    if (port > 0)
    {
        inet4_address addr(address_type::any, port);
        auto sock_ptr = std::make_shared<tcp4_socket>();

        socket::error_code ec = sock_ptr->bind(addr);

        if (ec == socket::error_code::success)
        {
            sock_ptr->listen();
            sock_ptr_ = sock_ptr;
            
            server_thread_ptr_ = std::make_unique<std::thread>(std::bind(&gui_server::server_thread, this));
            cleanup_thread_ptr_ = std::make_unique<std::thread>(std::bind(&gui_server::cleanup_thread, this));
        }
        else
        {
            logger::instance().error("gui_server: TCP port " +
                std::to_string(port) + " is busy");
        }
    }
}

gui_server::~gui_server()
{
    shutdown_ = true;
    abort_event_.signal();

    sock_ptr_->close();
    server_thread_ptr_->join();
    cleanup_thread_ptr_->join();

    //logger::instance().debug("gui_server destroyed");
}

bool gui_server::add_context(gui_context_ptr context_ptr)
{
    if (context_ptr)
    {
        std::lock_guard<std::mutex> lock(list_lock_);
        context_list_.push_back(context_ptr);
    }
    else
    {
        return false;
    }

    return context_ptr->start();
}

void gui_server::server_thread()
{
    DEBUG_REQUIRE(!!sock_ptr_);

    while (!shutdown_)
    {
        auto ec = sock_ptr_->wait_for_readable(100ms);

        if (ec == socket::error_code::success)
        {
            // accept incoming connection
            auto sock_ptr = sock_ptr_->accept();

            if (!!sock_ptr)
            {
                auto addr_ptr = sock_ptr->get_peer_address();
                sock_ptr->set_keep_alive(true);
                sock_ptr->set_no_delay(true);

                gui_context_ptr context_ptr = std::make_shared<gui_context>(sock_ptr,
                    std::bind(&gui_server::on_abort, this));
                
                if (add_context(context_ptr))
                {
                    logger::instance().debug("GUI client connected: " + addr_ptr->to_string());
                }
            }
        }
    }
}

void gui_server::cleanup_thread()
{
    while (!shutdown_)
    {
        abort_event_.wait();

        if (shutdown_)
        {
            break;
        }

        std::lock_guard<std::mutex> lock(list_lock_);
        std::list<gui_context_ptr> tmp_list;

        for (auto p : context_list_)
        {
            if (!p->is_aborted())
            {
                tmp_list.push_back(p);
            }
        }

        if (tmp_list.size() != context_list_.size())
        {
            context_list_.swap(tmp_list);
        }
    }
}

bool gui_server::on_bar(symbol sym, timeframe_type tf, const bar_data& bar)
{
    std::lock_guard<std::mutex> lock(list_lock_);
    int add_count = 0;

    if (!context_list_.empty())
    {
        for (auto p : context_list_)
        {
            if (!p->is_aborted())
            {
                if (p->add_bar(sym, tf, bar))
                {
                    add_count++;
                }
            }
        }
    }

    return add_count > 0;
}

bool gui_server::on_tick(symbol sym, const tick_data& tick)
{
    std::lock_guard<std::mutex> lock(list_lock_);
    int add_count = 0;

    if (!context_list_.empty())
    {
        for (auto p : context_list_)
        {
            if (!p->is_aborted())
            {
                if (p->add_tick(sym, tick))
                {
                    add_count++;
                }
            }
        }
    }

    return add_count > 0;
}

bool gui_server::on_order(order_cptr optr, order_action action)
{
    //DEBUG_TRACE("gui_server::on_order()");
    int add_count = 0;

    if (true) // scope
    {
        std::lock_guard<std::mutex> lock(list_lock_);

        if (!context_list_.empty())
        {
            for (auto p : context_list_)
            {
                if (!p->is_aborted())
                {
                    if (p->add_order(optr, action))
                    {
                        add_count++;
                    }
                }
            }
        }
    }

    save_order(optr, action);
    return add_count > 0;
}

bool gui_server::on_info(symbol sym, const std::string& xml)
{
    if (true) // scope
    {
        std::lock_guard<std::mutex> lock(list_lock_);

        if (!context_list_.empty())
        {
            for (auto p : context_list_)
            {
                if (!p->is_aborted())
                {
                    p->add_info(sym, xml);
                }
            }
        }
    }

    save_info(sym, xml);
    return true;
}

std::string gui_server::get_info(symbol sym)
{
    std::lock_guard<std::mutex> lock(info_lock_);    
    auto it = info_.find(sym);
    return (it != info_.end()) ? it->second : "";
}

void gui_server::save_order(order_cptr optr, order_action action)
{
    //DEBUG_TRACE("ID=%ld (%d)", optr->get_id(), action);
    std::lock_guard<std::mutex> lock(orders_lock_);

    symbol sym = optr->get_symbol();
    auto it = orders_.find(sym);

    if (it != orders_.end())
    {
        auto& v = it->second;
        v.push_back({ optr, action });
    }
    else
    {
        std::vector<order_info> v = {{ optr, action }};
        orders_.insert({ sym, v });
    }
}

void gui_server::save_info(symbol sym, const std::string& info_xml)
{
    std::lock_guard<std::mutex> lock(info_lock_);
    auto it = info_.find(sym);

    if (it != info_.end())
    {
        it->second = info_xml;
    }
    else
    {
        info_.insert({ sym, info_xml });
    }
}

void gui_server::get_orders(symbol sym, std::vector<order_info>& orders)
{
    std::vector<order_info> tmp;

    if (true) // scope
    {
        std::lock_guard<std::mutex> lock(orders_lock_);
        auto it = orders_.find(sym);

        if (it != orders_.end())
        {
            auto& v = it->second;

            if (!v.empty())
            {
                tmp.assign(v.begin(), v.end());
            }            
        }
    }

    orders.swap(tmp);
}

bool gui_server::get_options(symbol sym, options& opt) const
{
    std::lock_guard<std::mutex> lock(options_lock_);
    auto it = options_.find(sym);

    if (it != options_.end())
    {
        opt = it->second;
        return true;
    }

    return false;
}

bool gui_server::set_options(symbol sym, const options& opt)
{
    std::lock_guard<std::mutex> lock(options_lock_);
    auto it = options_.find(sym);

    if (it != options_.end())
    {
        it->second = opt;
    }
    else
    {
        options_.insert({ sym, opt });
    }

    return true;
}

} // namespace fx
