#pragma once
#include <map>
#include <mutex>
#include <vector>
#include "types.h"
#include "bar_data.h"

namespace fx {

class bar_collector
{
public:
    ~bar_collector();

    void put_bar(timeframe_type tf, const bar_data& bar);

    bool get_bars(timeframe_type tf, bar_array_type& bars) const;
    bool get_bars(timeframe_type tf, size_t start_index, size_t count, bar_array_type& bars) const;
    bool get_last_bars(timeframe_type tf, size_t count, bar_array_type& bars) const;
    bool get_bar_data(timeframe_type tf, bar_field field, size_t start_index, size_t count, data_array_type& data) const;

    bool empty(timeframe_type tf) const;
    size_t count(timeframe_type tf) const;

    void clear();
    bool clear(timeframe_type tf);

private:
    mutable std::mutex lock_;
    std::map<timeframe_type, std::vector<bar_data> > bars_;
};

} // namespace fx
