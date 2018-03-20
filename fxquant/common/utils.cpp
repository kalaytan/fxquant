#include <ctime>
#include <chrono>
#include "utils.h"

namespace fx {
const std::string time_to_string(fx::timepoint_type t)
{
    char time_str[20];
    time_t time = std::chrono::system_clock::to_time_t(t);
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", gmtime(&time));
    return time_str;
}

}