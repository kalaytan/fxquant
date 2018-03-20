#pragma once
#include <vector>
#include <string>
#include <sstream>
#include "debug.h"

namespace fx {

class info_data {
public:
    info_data() :
        counter_(0)
    {}

    template<typename T>
    void add_line(const std::string& key, const T& t)
    {
        std::ostringstream oss;
        oss.precision(7);
        oss << t;
        info_.push_back({ key, oss.str() });
    };

    void reset()
    {
        counter_++;
        std::vector<std::pair<std::string, std::string>> empty;
        info_.swap(empty);
    }

    bool is_empty() const
    {
        return  info_.empty();
    }

    std::string make_xml() const
    {
        //DEBUG_TRACE("counter_=%lu", counter_);
        std::ostringstream oss;
        oss << "<message id=\"info\">\n";
        oss << "<id>" << counter_ << "</id>\n";

        for (const auto& pair : info_)
        {
            oss << "  <" << pair.first << ">" << pair.second << "</" << pair.first << ">\n";
        }
        oss << "</message>";
        return oss.str();
    }
private:
    std::vector<std::pair<std::string, std::string>> info_;
    size_t counter_;
};
}