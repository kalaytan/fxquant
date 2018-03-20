#pragma once
#include <map>
#include <mutex>
#include <vector>
#include "symbol.h"
#include "fx_engine.h"

namespace fx {

class engine_registry // singleton
{
public:
    static engine_registry& instance()
    {
        static engine_registry instance_;
        return instance_;
    }

    // delete copy and move constructors and assign operators
    engine_registry(engine_registry const&) = delete;
    engine_registry(engine_registry&&) = delete;
    engine_registry& operator=(engine_registry const&) = delete;
    engine_registry& operator=(engine_registry &&) = delete;

    bool add_engine(engine_ptr eptr);
    engine_ptr get_engine(symbol sym) const;
    void get_engines(std::vector<engine_ptr>& v) const;

private:
    engine_registry() = default;

private:
    mutable std::mutex lock_;
    std::map<symbol, engine_ptr> engines_;
};

} // namespace fx
