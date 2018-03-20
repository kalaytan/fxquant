#pragma once
#include <ctime>

namespace fx {

struct bar_data
{
    double o;
    double h;
    double l;
    double c;
    time_t t;
};

enum class bar_field { o, h, l, c, t };

} // namespace fx
