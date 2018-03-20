#pragma once
#include "types.h"
#include "debug.h"
#include "event_queue.h"
#include "data_callback.h"

namespace fx {

class base_data_event : public base_event
{
public:
    base_data_event(data_callback_ptr cb_ptr) :
        callback_ptr_(cb_ptr)
    {
    }

protected:
    const data_callback_ptr callback_ptr_;
};

class tick_event : public base_data_event
{
public:
    tick_event(const tick_data& tick, data_callback_ptr cb_ptr) :
        base_data_event(cb_ptr), tick_(tick)
    {
    }

    void operator ()() override
    {
        if (callback_ptr_)
        {
            callback_ptr_->on_tick(tick_);
        }
    }

private:
    const tick_data tick_;
};

class bar_event : public base_data_event
{
public:
    bar_event(timeframe_type tf, const bar_data& bar, data_callback_ptr cb_ptr) :
        base_data_event(cb_ptr), time_frame_(tf), bar_(bar)
    {
    }

    void operator ()() override
    {
        if (callback_ptr_)
        {
            callback_ptr_->on_bar(time_frame_, bar_);
        }
    }

private:
    const timeframe_type time_frame_;
    const bar_data bar_;
};

class data_event_queue : public event_queue
{
public:
    explicit data_event_queue(data_callback_ptr dcb_ptr, size_t nthreads = 1) :
        event_queue(nthreads), dcb_ptr_(dcb_ptr)
    {
    }

    ~data_event_queue()
    {
        DEBUG_TRACE("~data_event_queue()");
    }

    bool push_tick_event(const tick_data& tick)
    {
        return push_event(std::make_shared<tick_event>(tick, dcb_ptr_));
    }

    bool push_bar_event(timeframe_type tf, const bar_data& bar)
    {
        return push_event(std::make_shared<bar_event>(tf, bar, dcb_ptr_));
    }

private:
    const data_callback_ptr dcb_ptr_;
};

} // namespace fx

