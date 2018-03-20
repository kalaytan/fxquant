#include "event.h"
#include "debug.h"

namespace fx {

event::~event()
{
    std::unique_lock<std::mutex> lock(mutex_);
}

void event::signal()
{
    std::unique_lock<std::mutex> lock(mutex_);
    signaled_ = true;
    cond_.notify_all();
}

bool event::wait(std::chrono::milliseconds timeout)
{
    std::unique_lock<std::mutex> lock(mutex_);

    if (signaled_ == false)
    {
        auto t = std::chrono::system_clock::now() + timeout;

        while (!signaled_)
        {
            if (cond_.wait_until(lock, t) == std::cv_status::timeout)
            {
                return false;
            }
        }
    }
    else
    {
        signaled_ = false;
    }

    return true;
}

bool event::wait()
{
    std::unique_lock<std::mutex> lock(mutex_);

    if (signaled_ == false)
    {
        cond_.wait(lock);
    }

    signaled_ = false;
    return true;
}
}
