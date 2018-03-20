#include "order.h"
#include "debug.h"
#include "message.h"
#include <sstream>
#include <stdexcept>

using namespace std::chrono;
using namespace fx::network;

namespace fx {

order_id_type base_order::auto_order_id_ = 0;

bool base_buy_order::is_valid() const
{
    if ((order_price_ == undefined_value<double>()) || (order_price_ <= 0.0) || (volume_ < 0.01) ||
        (has_stop_loss() && (stop_loss_ >= order_price_)) ||
        (has_take_profit() && (take_profit_ <= order_price_)))
    {
        return false;
    }

    return true;
}

double base_buy_order::get_profit() const
{
    if (!is_closed())
    {
        DEBUG_ASSERT(0); // use get_profit(tick) instead?
        return undefined_value<double>();
    }
    return (close_price_ - open_price_) / symbol_pip(symbol_) * volume_;
}

double base_buy_order::get_profit(const tick_data& tick) const
{
    return (tick.get_bid() - open_price_) / symbol_pip(symbol_) * volume_;
}

bool base_sell_order::is_valid() const
{
    if ((order_price_ == undefined_value<double>()) || (order_price_ <= 0.0) || (volume_ < 0.01) ||
        (has_stop_loss() && (stop_loss_ <= order_price_)) ||
        (has_take_profit() && (take_profit_ >= order_price_)))
    {
        return false;
    }

    return true;
}

double base_sell_order::get_profit() const
{
    if (!is_closed())
    {
        return undefined_value<double>();
    }
    return (open_price_ - close_price_) / symbol_pip(symbol_) * volume_;
}

double base_sell_order::get_profit(const tick_data& tick) const
{
    return (open_price_ - tick.get_ask()) / symbol_pip(symbol_) * volume_;
}

buy_order::buy_order(symbol sym, double volume, double order_price,
    double stop_loss, double take_profit, order_id_type id) :
    base_buy_order(sym, volume, order_price, stop_loss, take_profit, id)
{
    if (!is_valid())
    {
        throw std::invalid_argument("buy_order::buy_order() - invalid arguments");
    }
}

buy_order::buy_order(symbol sym, double volume, double order_price,
    int stop_loss, int take_profit, order_id_type id) :
    base_buy_order(sym, volume, order_price, stop_loss, take_profit, id)
{
    if (!is_valid())
    {
        throw std::invalid_argument("buy_order::buy_order() - invalid arguments");
    }
}

buy_limit_order::buy_limit_order(symbol sym, double volume, double order_price,
    double stop_loss, double take_profit, order_id_type id) :
    base_buy_order(sym, volume, order_price, stop_loss, take_profit, id)
{
    if (!is_valid())
    {
        throw std::invalid_argument("buy_limit_order::buy_limit_order() - invalid arguments");
    }
}

buy_limit_order::buy_limit_order(symbol sym, double volume, double order_price,
    int stop_loss, int take_profit, order_id_type id) :
    base_buy_order(sym, volume, order_price, stop_loss, take_profit, id)
{
    if (!is_valid())
    {
        throw std::invalid_argument("buy_limit_order::buy_limit_order() - invalid arguments");
    }
}

buy_stop_order::buy_stop_order(symbol sym, double volume, double order_price,
    double stop_loss, double take_profit, order_id_type id) :
    base_buy_order(sym, volume, order_price, stop_loss, take_profit, id)
{
    if (!is_valid())
    {
        throw std::invalid_argument("buy_stop_order::buy_stop_order() - invalid arguments");
    }
}

buy_stop_order::buy_stop_order(symbol sym, double volume, double order_price,
    int stop_loss, int take_profit, order_id_type id) :
    base_buy_order(sym, volume, order_price, stop_loss, take_profit, id)
{
    if (!is_valid())
    {
        throw std::invalid_argument("buy_stop_order::buy_stop_order() - invalid arguments");
    }
}

sell_order::sell_order(symbol sym, double volume, double order_price,
    double stop_loss, double take_profit, order_id_type id) :
    base_sell_order(sym, volume, order_price, stop_loss, take_profit, id)
{
    if (!is_valid())
    {
        throw std::invalid_argument("sell_order::sell_order() - invalid arguments");
    }
}

sell_order::sell_order(symbol sym, double volume, double order_price,
    int stop_loss, int take_profit, order_id_type id) :
    base_sell_order(sym, volume, order_price, stop_loss, take_profit, id)
{
    if (!is_valid())
    {
        throw std::invalid_argument("sell_order::sell_order() - invalid arguments");
    }
}

sell_limit_order::sell_limit_order(symbol sym, double volume, double order_price,
    double stop_loss, double take_profit, order_id_type id) :
    base_sell_order(sym, volume, order_price, stop_loss, take_profit, id)
{
    if (!is_valid())
    {
        throw std::invalid_argument("sell_limit_order::sell_limit_order() - invalid arguments");
    }
}

sell_limit_order::sell_limit_order(symbol sym, double volume, double order_price,
    int stop_loss, int take_profit, order_id_type id) :
    base_sell_order(sym, volume, order_price, stop_loss, take_profit, id)
{
    if (!is_valid())
    {
        throw std::invalid_argument("sell_limit_order::sell_limit_order() - invalid arguments");
    }
}

sell_stop_order::sell_stop_order(symbol sym, double volume, double order_price,
    double stop_loss, double take_profit, order_id_type id) :
    base_sell_order(sym, volume, order_price, stop_loss, take_profit, id)
{
    if (!is_valid())
    {
        throw std::invalid_argument("sell_stop_order::sell_stop_order() - invalid arguments");
    }
}

sell_stop_order::sell_stop_order(symbol sym, double volume, double order_price,
    int stop_loss, int take_profit, order_id_type id) :
    base_sell_order(sym, volume, order_price, stop_loss, take_profit, id)
{
    if (!is_valid())
    {
        throw std::invalid_argument("sell_stop_order::sell_stop_order() - invalid arguments");
    }
}

std::string buy_order::make_xml_header() const
{
    std::ostringstream oss;
    oss << "<order id=\"" << id_ << "\" type=\"buy_order\">\n";
    return oss.str();
}

std::string buy_limit_order::make_xml_header() const
{
    std::ostringstream oss;
    oss << "<order id=\"" << id_ << "\" type=\"buy_limit_order\">\n";
    return oss.str();
}

std::string buy_stop_order::make_xml_header() const
{
    std::ostringstream oss;
    oss << "<order id=\"" << id_ << "\" type=\"buy_stop_order\">\n";
    return oss.str();
}

std::string sell_order::make_xml_header() const
{
    std::ostringstream oss;
    oss << "<order id=\"" << id_ << "\" type=\"sell_order\">\n";
    return oss.str();
}

std::string sell_limit_order::make_xml_header() const
{
    std::ostringstream oss;
    oss << "<order id=\"" << id_ << "\" type=\"sell_limit_order\">\n";
    return oss.str();
}

std::string sell_stop_order::make_xml_header() const
{
    std::ostringstream oss;
    oss << "<order id=\"" << id_ << "\" type=\"sell_stop_order\">\n";
    return oss.str();
}

std::string base_order::to_xml_message(order_action action) const
{
    std::ostringstream oss;
    oss.precision(7);

    const std::string indent1(2, ' ');
    const std::string indent2(4, ' ');

    oss << "<message id=\"order\">\n";
    oss << indent1 << make_xml_header();

    if (action != order_action::undefined)
    {
        oss << indent2 << "<action>";

        switch (action)
        {
        case order_action::submitted:
            oss << "submitted"; break;
        case order_action::opened:
            oss << "opened"; break;
        case order_action::closed:
            oss << "closed"; break;
        case order_action::modified:
            oss << "modified"; break;
        case order_action::deleted:
            oss << "deleted"; break;
        }

        oss << "</action>\n";
    }

    oss << indent2 << "<symbol>" << symbol_to_string(symbol_) << "</symbol>\n";

    if ((volume_ > 0) && (volume_ != undefined_value<double>()))
    {
        oss << indent2 << "<volume>" << volume_ << "</volume>\n";
    }

    oss << indent2 << "<order_price>" << order_price_ << "</order_price>\n";

    if ((open_price_ > 0) && (open_price_ != undefined_value<double>()))
    {
        oss << indent2 << "<open_price>" << open_price_ << "</open_price>\n";
    }

    if ((action != order_action::opened) && (close_price_ > 0) && (close_price_ != undefined_value<double>()))
    {
        oss << indent2 << "<close_price>" << close_price_ << "</close_price>\n";
    }

    if (stop_loss_ != undefined_value<double>())
    {
        oss << indent2 << "<stop_loss>" << stop_loss_ << "</stop_loss>\n";
    }

    if (take_profit_ != undefined_value<double>())
    {
        oss << indent2 << "<take_profit>" << take_profit_ << "</take_profit>\n";
    }

    if (open_time_.time_since_epoch().count() > 0)
    {
        auto ms = duration_cast<milliseconds>(open_time_.time_since_epoch()).count();
        oss << indent2 << "<open_time>" << ms << "</open_time>\n";
    }

    if ((action != order_action::opened) && (close_time_.time_since_epoch().count() > 0))
    {
        auto ms = duration_cast<milliseconds>(close_time_.time_since_epoch()).count();
        oss << indent2 << "<close_time>" << ms << "</close_time>\n";
    }

    if (!comment_.empty())
    {
        oss << indent2 << "<comment>" << comment_ << "</comment_>\n";
    }

    oss << indent1 << "</order>\n";
    oss << "</message>";

    return oss.str();
}

order_ptr from_xml_message(const std::string& xml, order_action& action)
{
    order_ptr optr;
    action = order_action::undefined;

    try
    {
        do // dummy loop
        {
            pugi::xml_document doc;
            std::string tmp_xml(xml);

            pugi::xml_parse_result res = doc.load_buffer_inplace(&tmp_xml[0], tmp_xml.size());

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

            if (!get_message_id(message_node, message_id) || (message_id != "order"))
            {
                break;
            }

            pugi::xml_node order_node = message_node.child("order");

            if (!order_node)
            {
                break;
            }

            std::string order_type;
            order_id_type order_id = 0;

            pugi::xml_node::attribute_iterator ai = order_node.attributes_begin();

            while (ai != order_node.attributes_end())
            {
                std::string name = ai->name();

                if (name == "id")
                {
                    order_id = ai->as_uint();
                }
                else if (name == "type")
                {
                    order_type = ai->as_string();
                }

                ++ai;
            }

            pugi::xml_node symbol_node = order_node.child("symbol");

            if (!symbol_node)
            {
                break;
            }

            auto symbol = symbol_from_string(symbol_node.child_value());

            pugi::xml_node action_node = order_node.child("action");

            if (action_node)
            {
                std::string s = action_node.child_value();

                if (s == "submitted")
                {
                    action = order_action::submitted;
                }
                else if (s == "opened")
                {
                    action = order_action::opened;
                }
                else if (s == "closed")
                {
                    action = order_action::closed;
                }
                else if (s == "modified")
                {
                    action = order_action::modified;
                }
                else if (s == "deleted")
                {
                    action = order_action::deleted;
                }
            }

            double volume = undefined_value<double>();
            double order_price = undefined_value<double>();

            if (!read_value(order_node, "volume", volume))
            {
                break;
            }

            if (!read_value(order_node, "order_price", order_price))
            {
                break;
            }

            double stop_loss = undefined_value<double>();
            read_value(order_node, "stop_loss", stop_loss);

            double take_profit = undefined_value<double>();
            read_value(order_node, "take_profit", take_profit);

            double open_price = undefined_value<double>();
            read_value(order_node, "open_price", open_price);

            double close_price = undefined_value<double>();
            read_value(order_node, "close_price", close_price);

            unsigned long long open_time = 0;
            read_value(order_node, "open_time", open_time);

            unsigned long long close_time = 0;
            read_value(order_node, "close_time", close_time);

            std::string comment;
            pugi::xml_node comment_node = order_node.child("comment");

            if (!!comment_node)
            {
                comment = comment_node.child_value();
            }

            if (order_type == "buy_order")
            {
                optr = std::make_shared<buy_order>(symbol, volume, order_price, stop_loss, take_profit, order_id);
            }
            else if (order_type == "buy_limit_order")
            {
                optr = std::make_shared<buy_limit_order>(symbol, volume, order_price, stop_loss, take_profit, order_id);
            }
            else if (order_type == "buy_stop_order")
            {
                optr = std::make_shared<buy_stop_order>(symbol, volume, order_price, stop_loss, take_profit, order_id);
            }
            else if (order_type == "sell_order")
            {
                optr = std::make_shared<sell_order>(symbol, volume, order_price, stop_loss, take_profit, order_id);
            }
            else if (order_type == "sell_limit_order")
            {
                optr = std::make_shared<sell_limit_order>(symbol, volume, order_price, stop_loss, take_profit, order_id);
            }
            else if (order_type == "sell_stop_order")
            {
                optr = std::make_shared<sell_stop_order>(symbol, volume, order_price, stop_loss, take_profit, order_id);
            }

            if (!optr)
            {
                break; // invalid type
            }

            //optr->set_id(order_id);

            if (open_price != undefined_value<double>())
            {
                optr->set_open_price(open_price);
            }

            if (close_price != undefined_value<double>())
            {
                optr->set_close_price(close_price);
            }

            if (open_time > 0)
            {
                timepoint_type t = timepoint_type(milliseconds(open_time));
                optr->set_open_time(t);
            }

            if (close_time > 0)
            {
                timepoint_type t = timepoint_type(milliseconds(close_time));
                optr->set_close_time(t);
            }

            if (!comment.empty())
            {
                optr->set_comment(comment);
            }
        } while (0);
    }
    catch (std::exception&)
    {
        return nullptr;
    }

    return optr;
}

} // namespace fx
