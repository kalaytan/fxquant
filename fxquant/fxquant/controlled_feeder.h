#pragma once
#include <atomic>
#include <thread>
#include <vector>
#include "event.h"
#include "symbol.h"
#include "mongodb.h"
#include "data_feeder.h"

namespace fx {

// reads the tick data for selected day from DB
class controlled_feeder : public data_feeder
{
public:
    controlled_feeder();

    virtual bool start() override;
    virtual bool stop() override;

    void wait_for_stop() const
    {
        stop_event_.wait();
    }

private:
    void run();
    void sleep(const tick_data& td);
    
    bool is_warming_up() const
    {
        return (lookback_minutes_ > 0) && (bar_count_ < lookback_minutes_);
    }

    virtual void on_bar(timeframe_type tf, const bar_data& bar) override;

private:
    const int day_;
    const int month_;
    const int year_;
    const int days_;
    const bool use_cache_;   
    int lookback_minutes_;
    int speed_factor_;

    std::atomic_bool running_;
    std::unique_ptr<std::thread> thread_ptr_;
    std::unique_ptr<mongocxx::cursor> cursor_ptr_;

    mutable event stop_event_;
    std::vector<tick_data> cache_;

    std::atomic<int> delay_;
    timepoint_type prev_time_;
    std::atomic<size_t> bar_count_;
};

typedef std::shared_ptr<controlled_feeder> controlled_feeder_ptr;

} // namespace fx