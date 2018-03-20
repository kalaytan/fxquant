#pragma once
#include <thread>
#include <vector>
#include "event.h"
#include "symbol.h"
#include "mongodb.h"
#include "data_feeder.h"

namespace fx {

// reads the tick data for selected day from DB
class dummy_feeder : public data_feeder
{
public:
    dummy_feeder();
    //dummy_feeder(symbol sym, int day, int month, int year, int days, bool use_cache = false);
    virtual bool start() override;
    virtual bool stop() override;

    void wait_for_stop() const
    {
        stop_event_.wait();
    }

private:
    void run();

private:
    const int day_;
    const int month_;
    const int year_;
    const int days_;
    const bool use_cache_;

    bool running_;
    std::unique_ptr<std::thread> thread_ptr_;
    std::unique_ptr<mongocxx::cursor> cursor_ptr_;

    mutable event stop_event_;
    std::vector<tick_data> cache_;
};

typedef std::shared_ptr<dummy_feeder> dummy_feeder_ptr;

} // namespace fx