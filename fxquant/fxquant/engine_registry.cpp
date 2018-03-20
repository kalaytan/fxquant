#include "engine_registry.h"

namespace fx {

bool engine_registry::add_engine(engine_ptr eptr)
{
    if (eptr)
    {
        std::lock_guard<std::mutex> lock(lock_);

        symbol sym = eptr->get_feeder()->get_symbol();
        auto it = engines_.find(sym);

        if (it != engines_.end())
        {
            it->second = eptr;
        }
        else
        {
            engines_.insert({ sym, eptr });
        }

        return true;
    }

    return false;
}

engine_ptr engine_registry::get_engine(symbol sym) const
{
    std::lock_guard<std::mutex> lock(lock_);
    auto it = engines_.find(sym);

    if (it != engines_.end())
    {
        return it->second;
    }

    return nullptr;
}

void engine_registry::get_engines(std::vector<engine_ptr>& v) const
{
    v.clear();
    std::lock_guard<std::mutex> lock(lock_);
    
    for (const auto& e : engines_)
    {
        v.push_back(e.second);
    }
}

} // namespace fx
