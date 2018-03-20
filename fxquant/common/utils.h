#pragma once
#include <string>
#include <memory>
#include "types.h"

namespace fx {
const std::string time_to_string(fx::timepoint_type t);

template<typename Base, typename T>
bool derived_from(const T& o)
{
    try
    {
        dynamic_cast<const Base&>(o);
        return true;
    }
    catch (std::bad_cast&)
    {
    }
    return false;
}

template<typename Base, typename T>
bool derived_from(std::shared_ptr<T> ptr)
{
    return !!dynamic_cast<const Base*>(ptr.get());
}

}