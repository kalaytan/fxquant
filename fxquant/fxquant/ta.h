#pragma once
#include "types.h"
#include <ta-lib/ta_func.h>

namespace fx {

enum class ma_algo { undefined = 0, sma, ema, wma, dema, tema, trima, kama, mama, t3 };
TA_MAType algo_to_ta(ma_algo algo);

bool calc_ma(const data_array_type& input, unsigned int period, ma_algo algo, size_t ma_count, data_array_type& ma);

} // namespace fx

