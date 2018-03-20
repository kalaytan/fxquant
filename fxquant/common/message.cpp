#include <chrono>
#include <sstream>
#include <cstdint>
#include "debug.h"
#include "message.h"

using namespace std::chrono;

namespace {
int percent_to_speed_factor(int percent)
{
    DEBUG_ASSERT((percent >= 0) && (percent <= 100));
    const int min_speed_factor = 1;
    const int max_speed_factor = 900;

    if ((percent >= 0) && (percent <= 10)) // 1..60
    {
        return (60 - 1) * (percent - 0) / (10 - 0) + 1;
    }
    else if ((percent > 10) && (percent <= 40)) // 60..300
    {
        return (300 - 60) * (percent - 10) / (40 - 10) + 60;
    }
    else if ((percent > 40) && (percent <= 60)) // 300..600
    {
        return (600 - 300) * (percent - 40) / (60 - 40) + 300;
    }
    else if ((percent > 60) && (percent <= 100)) // 600..900
    {
        return (900 - 600) * (percent - 60) / (100 - 60) + 600;
    }

    return 0; // error
}
}

namespace fx {
namespace network {

std::string ping_message::to_xml() const
{
    std::ostringstream oss;
    oss << "<message id=\"ping\">\n";
    oss << "</message>";
    return oss.str();
}

bool ping_message::from_xml(const pugi::xml_node& message_node)
{
    DEBUG_ASSERT(!!message_node);
    return true;
}

std::string pong_message::to_xml() const
{
    std::ostringstream oss;
    oss << "<message id=\"pong\">\n";
    oss << "</message>";
    return oss.str();
}

bool pong_message::from_xml(const pugi::xml_node& message_node)
{
    DEBUG_ASSERT(!!message_node);
    return true;
}

std::string bar_message::to_xml() const
{
    std::ostringstream oss;
    oss.precision(7);

    oss << "<message id=\"bar\">\n";
    oss << "  <symbol>" << symbol_to_string(symbol_) << "</symbol>\n";
    oss << "  <time_frame>" << time_frame_.count() << "</time_frame>\n";
    oss << "  <bar_data>\n";
    oss << "    <o>" << bar_.o << "</o>\n";
    oss << "    <h>" << bar_.h << "</h>\n";
    oss << "    <l>" << bar_.l << "</l>\n";
    oss << "    <c>" << bar_.c << "</c>\n";
    oss << "    <t>" << bar_.t << "</t>\n";
    oss << "  </bar_data>\n";
    oss << "</message>";
    return oss.str();
}

bool bar_message::from_xml(const pugi::xml_node& message_node)
{
    DEBUG_ASSERT(!!message_node);

    pugi::xml_node symbol_node = message_node.child("symbol");

    if (!symbol_node)
    {
        return false;
    }

    symbol_ = symbol_from_string(symbol_node.child_value());

    unsigned int tf = 0;
        
    if (!read_value(message_node, "time_frame", tf))
    {
        return false;
    }

    time_frame_ = std::chrono::minutes(tf);
        
    pugi::xml_node bar_node = message_node.child("bar_data");

    if (!bar_node)
    {
        return false;
    }
        
    if (!read_value(bar_node, "o", bar_.o) ||
        !read_value(bar_node, "h", bar_.h) ||
        !read_value(bar_node, "l", bar_.l) ||
        !read_value(bar_node, "c", bar_.c) ||
        !read_value(bar_node, "t", bar_.t))
    {
        return false;
    }

    return true;
}

std::string tick_message::to_xml() const
{
    std::ostringstream oss;
    oss.precision(7);

    oss << "<message id=\"tick\">\n";
    oss << "  <symbol>" << symbol_to_string(symbol_) << "</symbol>\n";    
    oss << "  <tick_data>\n";
    oss << "    <bid>" << tick_.get_bid() << "</bid>\n";
    oss << "    <ask>" << tick_.get_ask() << "</ask>\n";

    auto epoch = tick_.get_time().time_since_epoch();
    oss << "    <time>" << duration_cast<milliseconds>(epoch).count() << "</time>\n";
    
    oss << "  </tick_data>\n";
    oss << "</message>";
    return oss.str();
}

bool tick_message::from_xml(const pugi::xml_node& message_node)
{
    DEBUG_ASSERT(!!message_node);

    pugi::xml_node symbol_node = message_node.child("symbol");

    if (!symbol_node)
    {
        return false;
    }

    symbol_ = symbol_from_string(symbol_node.child_value());

    pugi::xml_node tick_node = message_node.child("tick_data");

    if (!tick_node)
    {
        return false;
    }

    double bid = 0.0;
    double ask = 0.0;
    unsigned long long time = 0;

    if (!read_value(tick_node, "bid", bid) ||
        !read_value(tick_node, "ask", ask) ||
        !read_value(tick_node, "time", time))
    {
        return false;
    }

    tick_data tick(bid, ask, timepoint_type(milliseconds(time)));
    tick_ = tick;

    return true;
}

std::string bar_array_message::to_xml() const
{
    std::ostringstream oss;
    oss << "<message id=\"bar_array\">\n";
    oss << "  <symbol>" << symbol_to_string(symbol_) << "</symbol>\n";
    oss << "  <time_frame>" << time_frame_.count() << "</time_frame>\n";
    oss << "  <count>" << count_ << "</count>\n";
    oss << "</message>";
    return oss.str();
}

bool bar_array_message::from_xml(const pugi::xml_node& message_node)
{
    DEBUG_ASSERT(!!message_node);

    pugi::xml_node symbol_node = message_node.child("symbol");

    if (!symbol_node)
    {
        return false;
    }

    symbol_ = symbol_from_string(symbol_node.child_value());

    unsigned int tf = 0;

    if (!read_value(message_node, "time_frame", tf))
    {
        return false;
    }

    time_frame_ = std::chrono::minutes(tf);

    unsigned int count = 0;

    if (!read_value(message_node, "count", count) || (count == 0))
    {
        return false;
    }

    count_ = count;

    return true;
}

std::string status_message::to_xml() const
{
    std::ostringstream oss;
    oss << "<message id=\"status\">\n";
    oss << "  <status>" << static_cast<int>(status_) << "</status>\n";
    oss << "</message>";
    return oss.str();
}

bool status_message::from_xml(const pugi::xml_node& message_node)
{
    DEBUG_ASSERT(!!message_node);

    unsigned int status_int = 0;

    if (!read_value(message_node, "status", status_int))
    {
        return false;
    }

    status_ = static_cast<status>(status_int);
    return true;
}

std::string options_message::to_xml() const
{
    std::ostringstream oss;
    oss << "<message id=\"options\">\n";

    if (symbol_ != symbol::undefined)
    {
        oss << "  <symbol>" << symbol_to_string(symbol_) << "</symbol>\n";
    }    

    oss << "  <speed>" << static_cast<int>(speed_) << "</speed>\n";
    oss << "</message>";
    
    return oss.str();
}

bool options_message::from_xml(const pugi::xml_node& message_node)
{
    DEBUG_ASSERT(!!message_node);

    pugi::xml_node symbol_node = message_node.child("symbol");

    if (symbol_node)
    {
        symbol_ = symbol_from_string(symbol_node.child_value());
    }
    else
    {
        symbol_ = symbol::undefined;
    }
    
    unsigned int percent = 0;

    if (read_value(message_node, "speed", percent))
    {
        speed_ = percent_to_speed_factor(percent);
    }

    return true;
}

bool get_message_id(pugi::xml_node& message_node, std::string& id)
{
    pugi::xml_node::attribute_iterator ai = message_node.attributes_begin();

    while (ai != message_node.attributes_end())
    {
        std::string name = ai->name();

        if (name == "id")
        {
            id = ai->as_string();
            return true;
        }

        ++ai;
    }

    return false;
}

bool read_value(pugi::xml_node parent_node, const std::string& child_name, double& value)
{
    try
    {
        pugi::xml_node child_node = parent_node.child(child_name.c_str());

        if (!!child_node)
        {
            std::string s = child_node.child_value();
            value = std::stod(s);
            return true;
        }
    }
    catch (...)
    {
    }

    return false;
}

bool read_value(pugi::xml_node parent_node, const std::string& child_name, unsigned int& value)
{
    try
    {
        pugi::xml_node child_node = parent_node.child(child_name.c_str());

        if (!!child_node)
        {
            std::string s = child_node.child_value();
            value = static_cast<unsigned int>(std::stoul(s));
            return true;
        }
    }
    catch (...)
    {
    }

    return false;
}

bool read_value(pugi::xml_node parent_node, const std::string& child_name, unsigned long long& value)
{
    try
    {
        pugi::xml_node child_node = parent_node.child(child_name.c_str());

        if (!!child_node)
        {
            std::string s = child_node.child_value();
            value = static_cast<time_t>(std::stoull(s));
            return true;
        }
    }
    catch (...)
    {
    }

    return false;
}

bool read_value(pugi::xml_node parent_node, const std::string& child_name, time_t& value)
{
    unsigned long long tmp = 0;
    bool res = read_value(parent_node, child_name, tmp);
    value = static_cast<time_t>(tmp);
    return res;
}

} // namespace network
} // namespace fx
