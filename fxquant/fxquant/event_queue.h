#pragma once
#include <queue>
#include <mutex>
#include <vector>
#include <thread>
#include "event.h"

namespace fx {

struct base_event
{
    virtual ~base_event() = default;
    virtual void operator ()() = 0;
};

typedef std::shared_ptr<base_event> base_event_ptr;

class event_queue
{
public:
    explicit event_queue(size_t nthreads);
    virtual ~event_queue();

    bool push_event(base_event_ptr eptr);

    size_t size() const
    {
        std::lock_guard<std::mutex> lock(lock_);
        return queue_.size();
    }

private:
    typedef std::shared_ptr<std::thread> thread_ptr;

    struct thread_context
    {
        thread_ptr thread_ptr_;
        bool terminated_;
    };

    typedef std::shared_ptr<thread_context> thread_context_ptr;

private:
    base_event_ptr pop_event();
    void thread_func(thread_context_ptr tc_ptr);

private:
    bool shutdown_;
    mutable std::mutex lock_;
    mutable event signal_;
    std::queue<base_event_ptr> queue_;
    std::vector<thread_context_ptr> threads_;
};

} // namespace fx
