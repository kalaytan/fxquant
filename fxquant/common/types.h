#pragma once
#include <limits>
#include <chrono>
#include <vector>
#include "bar_data.h"

namespace fx {

template <typename T>
constexpr T undefined_value()
{
    #if defined(min) // disable warning
    #undef min
    #endif
    return std::numeric_limits<T>::min();
}

template <>
constexpr double undefined_value()
{
    return std::numeric_limits<double>::infinity();
}

typedef std::chrono::time_point<std::chrono::system_clock> timepoint_type;
typedef std::chrono::minutes timeframe_type;

typedef std::vector<double> data_array_type;
typedef std::vector<bar_data> bar_array_type;

struct ohlct
{
    std::vector<double> vo;
    std::vector<double> vh;
    std::vector<double> vl;
    std::vector<double> vc;
    std::vector<double> vt;
};

typedef std::chrono::milliseconds timeout_type;

enum class status
{
    undefined   = 0,
    initialized = 1
};

} // namespace fx
