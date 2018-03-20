#include "event_queue.h"

using namespace std::chrono_literals;

namespace fx {

event_queue::event_queue(size_t nthreads) : shutdown_(false)
{
    for (size_t i = 0; i < nthreads; i++)
    {
        auto ctx_ptr = std::make_shared<thread_context>();
        ctx_ptr->terminated_ = false;

        ctx_ptr->thread_ptr_ = std::make_shared<std::thread>(
            std::bind(&event_queue::thread_func, this, ctx_ptr));

        threads_.push_back(ctx_ptr);
    }
}

event_queue::~event_queue()
{
    { // scope
        std::lock_guard<std::mutex> lock(lock_);
        shutdown_ = true;
    }

    while (!threads_.empty())
    {
        signal_.signal();
        std::vector<thread_context_ptr> running_threads;

        for (auto ctx_ptr : threads_)
        {
            if (ctx_ptr->terminated_)
            {
                ctx_ptr->thread_ptr_->join();
            }
            else
            {
                running_threads.push_back(ctx_ptr);
            }
        }

        threads_.swap(running_threads);

        if (!threads_.empty())
        {
            std::this_thread::sleep_for(10ms); // short delay
        }
    }
}

bool event_queue::push_event(base_event_ptr eptr)
{
    if (!shutdown_)
    {
        std::lock_guard<std::mutex> lock(lock_);

        if (eptr && !shutdown_)
        {
            queue_.push(eptr);
            signal_.signal();
            return true;
        }
    }

    return false;
}

base_event_ptr event_queue::pop_event()
{
    if (!shutdown_)
    {
        std::lock_guard<std::mutex> lock(lock_);

        if (!queue_.empty() && !shutdown_)
        {
            auto eptr = queue_.front();
            queue_.pop();
            return eptr;
        }
    }

    return nullptr;
}

void event_queue::thread_func(thread_context_ptr tc_ptr)
{
    while (!shutdown_)
    {
        base_event_ptr eptr = pop_event();

        if (eptr)
        {
            (*eptr)(); // execute
        }
        else
        {
            signal_.wait(100ms);
        }
    }

    tc_ptr->terminated_ = true;
}

} // namespace fx
