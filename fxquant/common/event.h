#pragma once
#include <mutex>
#include <chrono>
#include <memory>
#include <condition_variable>

namespace fx {

class event
{
public:
    event() : signaled_(false) {}
    ~event();

    void signal();
    bool wait();
    bool wait(std::chrono::milliseconds timeout);

private:
    mutable std::condition_variable cond_;
    mutable std::mutex mutex_;
    bool signaled_;
};

typedef std::shared_ptr<event> event_ptr;
}
