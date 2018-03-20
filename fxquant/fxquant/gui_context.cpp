#include <atomic>
#include "socket.h"
#include "logger.h"
#include "message.h"
#include "gui_context.h"
#include "engine_registry.h"

namespace {
bool ticks_equal(const fx::tick_data& a, const fx::tick_data& b)
{
    const int scale = 1000;
    
    int a_ask = static_cast<int>(a.get_ask() * scale);
    int b_ask = static_cast<int>(b.get_ask() * scale);

    if (a_ask != b_ask)
    {
        return false;
    }

    int a_bid = static_cast<int>(a.get_bid() * scale);
    int b_bid = static_cast<int>(b.get_bid() * scale);

    if (a_bid != b_bid)
    {
        return false;
    }

    return true;
}
}

namespace fx {

using namespace fx::network;
using namespace std::chrono;
using namespace std::chrono_literals;

gui_context::gui_context(network::tcp_socket_ptr sock_ptr, callback_func on_abort) :
    sock_ptr_(sock_ptr), on_abort_(on_abort), aborted_(false), canceled_(false), bar_count_(0)
{
    started_.clear();    
}

gui_context::~gui_context()
{
    canceled_ = true;
    data_event_.signal();
    sock_ptr_->close();

    if (started_.test_and_set())
    {
        send_thread_ptr_->join();
        recv_thread_ptr_->join();
    }

    logger::instance().debug("GUI client disconnected");
}

bool gui_context::start()
{
    if (!started_.test_and_set())
    {
        send_thread_ptr_ = std::make_unique<std::thread>(std::bind(&gui_context::send_thread_func, this));
        recv_thread_ptr_ = std::make_unique<std::thread>(std::bind(&gui_context::recv_thread_func, this));
        return true;
    }

    return false;
}

bool gui_context::add_bar(symbol sym, timeframe_type tf, const bar_data& bar)
{
    bool added = false;

    std::lock_guard<std::mutex> lock(data_lock_);
    auto i1 = state_map_.find(sym);

    if (i1 != state_map_.end())
    {        
        auto& m = i1->second.bars_; // map timeframe -> vector<bar_data_int>
        auto i2 = m.find(tf);

        if (i2 != m.end())
        {
            auto& v = i2->second; // vector<bar_data_int>

            if (v.empty() || (v.rbegin()->t < bar.t))
            {
                v.push_back(bar);
                added = true;
            }
        }
        else // add new timeframe
        {
            std::vector<bar_data> v = { bar };
            i1->second.bars_.insert({tf, v});
            added = true;
        }
    }
    else // add new symbol
    {
        state new_state;
        std::vector<bar_data> v = { bar };
        new_state.bars_.insert({ tf, v });
        state_map_.insert({ sym, new_state });
        added = true;
    }

    if (added)
    {
        data_event_.signal();
    }

    return added;
}

bool gui_context::add_bars(symbol sym, timeframe_type tf, const std::vector<bar_data>& bars)
{
    bool added = false;

    if (!bars.empty())
    {
        std::lock_guard<std::mutex> lock(data_lock_);
        auto i1 = state_map_.find(sym);

        if (i1 != state_map_.end())
        {
            auto& m = i1->second.bars_; // map timeframe -> vector<bar_data>
            auto i2 = m.find(tf);

            if (i2 != m.end())
            {
                auto& v = i2->second; // vector<bar_data>

                if (!v.empty())
                {
                    auto to_time = v.begin()->t;

                    for (auto j = bars.begin(); j != bars.end(); ++j)
                    {
                        if (j->t >= to_time)
                        {
                            v.insert(v.begin(), bars.begin(), j);
                            added = true;
                            break;
                        }
                    }
                }
                else
                {
                    v.insert(v.begin(), bars.begin(), bars.end());
                    added = true;
                }                
            }
            else // add new timeframe
            {
                i1->second.bars_.insert({ tf, bars });
                added = true;
            }
        }
        else // add new symbol
        {
            state new_state;
            new_state.bars_.insert({ tf, std::vector<bar_data>() });
            auto p = state_map_.insert({ sym, new_state });
            auto it = p.first->second.bars_.begin();
            auto& v = it->second; // vector<bar_data>
            v.insert(v.end(), bars.begin(), bars.end());
            added = true;
        }
    }

    if (added)
    {
        data_event_.signal();
    }

    return added;
}

bool gui_context::add_tick(symbol sym, const tick_data& tick)
{
    bool added = false;

    std::lock_guard<std::mutex> lock(data_lock_);
    auto i1 = state_map_.find(sym);

    if (i1 != state_map_.end())
    {
        auto& v = i1->second.ticks_;

        if (!v.empty())
        {
            auto& last = *(v.rbegin());

            if ((last.get_time() < tick.get_time()) && !ticks_equal(last, tick))
            {
                v.clear();
                v.push_back(tick);
                added = true;
            }
        }
        else
        {
            v.push_back(tick);
            added = true;
        }
    }
    else // add new symbol
    {
        state new_state;
        new_state.ticks_ = { tick };
        state_map_.insert({ sym, new_state });
        added = true;
    }

    if (added)
    {
        data_event_.signal();
    }

    return added;
}

bool gui_context::add_order(order_cptr optr, order_action action)
{
    bool added = false;

    std::lock_guard<std::mutex> lock(data_lock_);
    auto i1 = state_map_.find(optr->get_symbol());

    if (i1 != state_map_.end())
    {
        auto& v = i1->second.orders_;
        v.push_back({ optr, action });
        added = true;
    }
    else // add new symbol
    {
        state new_state;
        new_state.orders_ = {{ optr, action }};
        state_map_.insert({ optr->get_symbol(), new_state });
        added = true;
    }

    if (added)
    {
        data_event_.signal();
    }

    return added;
}

bool gui_context::add_orders(symbol sym)
{
    std::lock_guard<std::mutex> lock(data_lock_);
    auto i1 = state_map_.find(sym);

    if (i1 != state_map_.end())
    {
        auto& v = i1->second.orders_;
        gui_server::instance().get_orders(sym, v);
    }
    else // add new symbol
    {
        state new_state;
        auto i2 = state_map_.insert({ sym, new_state });
        auto& v = i2.first->second.orders_;
        gui_server::instance().get_orders(sym, v);
    }

    return true;
}

bool gui_context::add_info(symbol sym, const std::string& info_xml)
{
    std::lock_guard<std::mutex> lock(data_lock_);
    auto i1 = state_map_.find(sym);

    if (i1 != state_map_.end())
    {
        i1->second.info_ = info_xml;
    }
    else // add new symbol
    {
        state new_state;
        auto i2 = state_map_.insert({ sym, new_state });
        i2.first->second.info_ = info_xml;
    }

    if (!info_xml.empty())
    {
        data_event_.signal();
    }
    
    return true;
}

bool gui_context::send_ticks()
{
    std::map<symbol, tick_data> ticks_to_send;

    if (true) // scope
    {
        std::lock_guard<std::mutex> lock(data_lock_);

        for (auto& e : state_map_)
        {
            auto& v = e.second.ticks_;

            if (!v.empty())
            {
                ticks_to_send.insert({ e.first, *(v.rbegin()) });
                v.clear();
            }
        }
    }

    if (ticks_to_send.empty())
    {
        return false;
    }

    for (auto& e : ticks_to_send)
    {
        tick_message tick;
        tick.symbol_ = e.first;
        tick.tick_ = e.second;

        if (!send_message(tick.to_xml()))
        {
            // network error
            DEBUG_ENSURE(aborted_ == true);
            return false;
        }
    }

    return true;
}

bool gui_context::send_bars()
{    
    bool success = false;
    symbol sym = symbol::undefined;
    timeframe_type tf;
    std::vector<bar_data> bars_to_send;

    if (true) // scope
    {
        std::lock_guard<std::mutex> lock(data_lock_);
        
        for (auto& a : state_map_)
        {
            auto& m = a.second.bars_;

            for (auto& b : m)
            {
                if (!b.second.empty())
                {
                    sym = a.first;
                    tf = b.first;
                    bars_to_send.swap(b.second);
                    break;
                }
            }
        }
    }
    
    if (bars_to_send.empty())
    {
        return false; // nothing to send
    }

    if (tf == 1min)
    {
        bar_count_ += bars_to_send.size();
        //logger::instance().debug("bar_count=" + std::to_string(bar_count_));
    }

    if (bars_to_send.size() > 1)
    {
        bar_array_message bar_array;
        bar_array.symbol_ = sym;
        bar_array.time_frame_ = tf;
        bar_array.count_ = bars_to_send.size();

        if (send_message(bar_array.to_xml()))
        {
            size_t bars_sent = 0;

            while (!canceled_ && !aborted_)
            {
                size_t bars_left = bars_to_send.size() - bars_sent;

                if (bars_left == 0)
                {
                    return true;
                }

                size_t n = bars_left >= 10 ? 10 : bars_left;
                const auto* p = &bars_to_send[0] + bars_sent;

                size_t bytes = 0;
                socket::error_code err = sock_ptr_->write(p, n * sizeof(bar_data), bytes, 8s);

                if ((err != socket::error_code::success) ||
                    (bytes != n * sizeof(bar_data)))
                {
                    break; // network error
                }

                bars_sent += n;
            }

            success = (bars_sent == bar_array.count_);
        }
    }
    else // size = 1
    {
        bar_message bar;
        bar.symbol_ = sym;
        bar.time_frame_ = tf;
        bar.bar_ = bars_to_send[0];

        success = send_message(bar.to_xml());
    }

    if (!success)
    {
        logger::instance().error("gui_context::send_bars() failed");
    }

    return success;
}

bool gui_context::send_orders()
{
    std::vector<std::pair<order_cptr, order_action>> orders_to_send;

    if (true) // scope
    {
        std::lock_guard<std::mutex> lock(data_lock_);

        for (auto& e : state_map_)
        {
            auto& v = e.second.orders_;

            if (!v.empty())
            {
                orders_to_send.swap(v);
                break;
            }
        }
    }

    if (orders_to_send.empty())
    {
        return false;
    }

    for (auto& o : orders_to_send)
    {
        auto xml = o.first->to_xml_message(o.second);
        //DEBUG_TRACE("%s", xml.c_str());

        if (!send_message(xml))
        {
            // network error
            DEBUG_ENSURE(aborted_ == true);
            return false;
        }
    }

    return true;
}

bool gui_context::send_info()
{
    std::string info_xml;

    if (true) // scope
    {
        std::lock_guard<std::mutex> lock(data_lock_);

        for (auto& e : state_map_)
        {
            if (!e.second.info_.empty())
            {
                info_xml.swap(e.second.info_);
                break;
            }
        }
    }

    if (info_xml.empty())
    {
        return false;
    }

    if (!send_message(info_xml))
    {
        // network error
        DEBUG_ENSURE(aborted_ == true);
        return false;
    }

    return true;
}

bool gui_context::send_status(status e)
{
    status_message sm;
    sm.status_ = e;

    if (!send_message(sm.to_xml()))
    {
        // network error
        DEBUG_ENSURE(aborted_ == true);
        return false;
    }

    return true;
}

void gui_context::send_thread_func()
{
    std::vector<engine_ptr> v;
    engine_registry::instance().get_engines(v);

    for (auto eptr : v)
    {
        initialize(eptr->get_symbol(), eptr->get_bar_collector());
    }

    std::atomic_flag initialized;
    initialized.clear();
    
    while (!canceled_ && !aborted_)
    {
        bool bars_sent = send_bars();
        bool orders_sent = send_orders();

        if (bars_sent || orders_sent)
        {
            if (!initialized.test_and_set())
            {
                // initialization done
                send_status(status::initialized);
            }

            continue; // some data sent
        }

        send_info();
        send_ticks();

        if (canceled_ || aborted_)
        {
            break;
        }

        // all data sent
        data_event_.wait(1s);
    }
}

void gui_context::recv_thread_func()
{
    while (!canceled_ && !aborted_)
    {
        if (sock_ptr_->wait_for_readable(100ms) != socket::error_code::success)
        {
            continue;
        }

        if (canceled_ || aborted_)
        {
            break;
        }

        std::string xml;
        
        if (!receive_message(xml, 4s))
        {
            break; // error
        }

        pugi::xml_document doc;

        pugi::xml_parse_result res = doc.load_buffer_inplace(&xml[0], xml.size());

        if (!res)
        {
            break;
        }

        pugi::xml_node message_node = doc.child("message");

        if (!message_node)
        {
            break;
        }

        std::string message_id;

        if (!get_message_id(message_node, message_id))
        {
            break;
        }

        if (message_id == "ping")
        {
            pong_message pong;
            send_message(pong.to_xml());
        }
        else if (message_id == "options")
        {
            options_message opts_msg;
            
            if (opts_msg.from_xml(message_node) && (opts_msg.symbol_ != symbol::undefined))
            {
                gui_server::options opts;
                opts.speed_factor_ = opts_msg.speed_;
                logger::instance().info("speed_factor changed to " + std::to_string(opts.speed_factor_));
                gui_server::instance().set_options(opts_msg.symbol_, opts);
            }
        }
    }
}

bool gui_context::initialize(symbol sym, const bar_collector& bc)
{
    const timeframe_type tf_arr[] = { 1min, 5min, 15min, 30min, 1h, 4h, 168h, 720h };

    for (auto tf : tf_arr)
    {
        if (!bc.empty(tf))
        {
            bar_array_type bars;
            
            if (bc.get_bars(tf, bars) && !bars.empty())
            {
                add_bars(sym, tf, bars);
            }
        }
    }

    add_orders(sym);

    std::string xml = gui_server::instance().get_info(sym);
    add_info(sym, xml);

    return true;
}

void gui_context::set_aborted()
{
    if (!aborted_)
    {
        aborted_ = true;

        if (on_abort_)
        {
            on_abort_();
        }
    }
}

bool gui_context::send_message(const std::string& xml)
{
    if (!canceled_ && !aborted_ && !xml.empty())
    {
        std::lock_guard<std::mutex> lock(send_lock_);
        
        size_t bytes = 0;

        if (sock_ptr_->wait_for_writable(1min) == socket::error_code::success)
        {
            auto err = sock_ptr_->write(&xml[0], xml.size(), bytes, 30s);

            if (err == socket::error_code::success)
            {
                return true;
            }
        }

        set_aborted();
    }

    return false;
}

bool gui_context::receive_message(std::string& xml, timeout_type timeout)
{
    const timeout_type zero_timeout = 0ms;
    const std::string closing_tag("</message>");
    xml.clear();

    auto time_left = timeout;
    auto time_start = system_clock::now();

    while (!canceled_ && !aborted_)
    {
        if (sock_ptr_->wait_for_readable(time_left) != socket::error_code::success)
        {
            break; // timeout expired
        }

        char buf[256] = { 0 };
        size_t bytes = 0;

        if (sock_ptr_->peek(buf, sizeof(buf), bytes) != socket::error_code::success)
        {
            break; // network error
        }

        xml.append(buf, bytes);
        auto i = xml.find(closing_tag);

        if (i != std::string::npos) // closing tag received
        {
            size_t tail_size = xml.size() - (i + closing_tag.size());
            DEBUG_ASSERT(bytes >= tail_size);

            // remove bytes from the socket buffer
            auto err = sock_ptr_->read(buf, bytes - tail_size, bytes, zero_timeout);

            if (err != socket::error_code::success)
            {
                break; // network error
            }

            xml.resize(xml.size() - tail_size);
            return true; // done
        }
        else
        {
            sock_ptr_->read(buf, bytes, bytes, zero_timeout); // remove bytes
        }

        auto time_now = system_clock::now();
        auto time_spent = time_now - time_start;

        if (time_spent >= timeout)
        {
            break; // timeout expired
        }

        time_left = duration_cast<milliseconds>(timeout - time_spent);
    }

    set_aborted();
    return false;
}

} // namespace fx
