#pragma once
#include <set>
#include <map>
#include <mutex>
#include <memory>
#include <vector>
#include "types.h"
#include "symbol.h"
#include "fixed_point.h"
#include "data_callback.h"
#include "candle_factory.h"

namespace fx {

class data_feeder // a base class for all feeders
{
public:
    explicit data_feeder(symbol sym);
    virtual ~data_feeder() = default;

    virtual bool start() = 0;
    virtual bool stop() = 0;

    symbol get_symbol() const
    {
        return symbol_;
    }

    bool add_callback(data_callback_ptr cb_ptr);
    bool remove_callback(data_callback_ptr cb_ptr);

    double normalize(double d) const;

protected:
    void on_tick(const tick_data& tick, bool ignore_callback = false);
    virtual void on_bar(timeframe_type tf, const bar_data& bar);

protected:
    const symbol symbol_;
    const int precision_;
    mutable std::mutex lock_;
    std::set<data_callback_ptr> callbacks_;
    std::map<timeframe_type, candle_factory> candle_factories_;
};

typedef std::shared_ptr<data_feeder> data_feeder_ptr;

} // namespace fx
