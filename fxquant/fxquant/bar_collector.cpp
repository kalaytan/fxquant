#include "debug.h"
#include "bar_collector.h"

namespace fx {

bar_collector::~bar_collector()
{
    DEBUG_TRACE("~bar_collector()");
}

void bar_collector::put_bar(timeframe_type tf, const bar_data& bar)
{
    std::lock_guard<std::mutex> lock(lock_);
    auto it = bars_.find(tf);

    if (it != bars_.end())
    {
        // add bar to existing queue
        it->second.push_back(bar);
    }
    else
    {
        bar_array_type q{ bar };
        bars_.insert({ tf, q });
    }
}

bool bar_collector::get_bars(timeframe_type tf, bar_array_type& bars) const
{
    std::lock_guard<std::mutex> lock(lock_);
    auto it = bars_.find(tf);

    if (it != bars_.end())
    {
        // copy bars
        const auto& v = it->second;
        bars.assign(v.begin(), v.end());
        return true;
    }

    bars.clear();
    return false;
}

bool bar_collector::get_bars(timeframe_type tf, size_t start_index, size_t count, bar_array_type& bars) const
{
    std::lock_guard<std::mutex> lock(lock_);
    auto it = bars_.find(tf);

    if (it != bars_.end())
    {
        const auto& v = it->second;

        if ((start_index + count) <= v.size())
        {
            // copy bars
            bars.assign(v.begin() + start_index, v.begin() + (start_index + count));
            return true;
        }
    }

    bars.clear();
    return false;
}

bool bar_collector::get_last_bars(timeframe_type tf, size_t count, bar_array_type& bars) const
{
    std::lock_guard<std::mutex> lock(lock_);
    auto it = bars_.find(tf);

    if (it != bars_.end())
    {
        const auto& v = it->second;

        if (count <= v.size())
        {
            // copy bars
            bars.assign(v.begin() + (v.size() - count), v.end());
            return true;
        }
    }

    bars.clear();
    return false;
}

bool bar_collector::get_bar_data(timeframe_type tf, bar_field field,
    size_t start_index, size_t count, data_array_type& data) const
{
    data.clear();

    if (field == bar_field::t)
    {
        return false;
    }

    std::lock_guard<std::mutex> lock(lock_);
    auto it = bars_.find(tf);

    if (it != bars_.end())
    {
        const auto& v = it->second;

        if ((start_index + count) > v.size())
        {
            return false;
        }

        data.resize(count);
        auto vit = v.begin() + start_index;

        for (auto dit = data.begin(); dit != data.end(); dit++, vit++)
        {
            switch (field)
            {
            case bar_field::c: *dit = vit->c; break;
            case bar_field::o: *dit = vit->o; break;
            case bar_field::h: *dit = vit->h; break;
            case bar_field::l: *dit = vit->l; break;
            default:
                return false;
            }
        }

        return true;
    }

    return false;
}

size_t bar_collector::count(timeframe_type tf) const
{
    std::lock_guard<std::mutex> lock(lock_);
    auto it = bars_.find(tf);

    if (it != bars_.end())
    {
        return it->second.size();
    }

    return 0;
}

bool bar_collector::empty(timeframe_type tf) const
{
    std::lock_guard<std::mutex> lock(lock_);
    auto it = bars_.find(tf);

    if (it != bars_.end())
    {
        return it->second.empty();
    }

    return true;
}

void bar_collector::clear()
{
    std::lock_guard<std::mutex> lock(lock_);
    bars_.clear();
}

bool bar_collector::clear(timeframe_type tf)
{
    std::lock_guard<std::mutex> lock(lock_);
    auto it = bars_.find(tf);

    if (it != bars_.end())
    {
        it->second.clear();
        return true;
    }

    return false;
}

} // namespace fx
