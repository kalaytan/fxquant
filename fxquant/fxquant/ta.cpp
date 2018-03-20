#include "ta.h"
#include "debug.h"

namespace fx {

TA_MAType algo_to_ta(ma_algo algo)
{
    switch (algo)
    {
    case ma_algo::sma:   return TA_MAType_SMA;
    case ma_algo::ema:   return TA_MAType_EMA;
    case ma_algo::wma:   return TA_MAType_WMA;
    case ma_algo::dema:  return TA_MAType_DEMA;
    case ma_algo::tema:  return TA_MAType_TEMA;
    case ma_algo::trima: return TA_MAType_TRIMA;
    case ma_algo::kama:  return TA_MAType_KAMA;
    case ma_algo::mama:  return TA_MAType_MAMA;
    case ma_algo::t3:    return TA_MAType_T3;
    }

    return TA_MAType_SMA;
}

bool calc_ma(const data_array_type& input, unsigned int period, ma_algo algo, size_t ma_count, data_array_type& ma)
{
    if (period < 2)
    {
        return false; // invalid argument
    }

    const size_t input_size = input.size();
    const size_t lookback = TA_MA_Lookback(period, algo_to_ta(algo));

    if ((lookback + ma_count) > input_size)
    {
        return false; // not enough data
    }

    int count = 0;
    int start_index = 0;
    ma.resize(ma_count);

    TA_RetCode rc = TA_MA(static_cast<int>(input_size - ma_count),
        static_cast<int>(input_size - 1), &input[0], period, algo_to_ta(algo),
        &start_index, &count, &ma[0]);

    if (rc != TA_SUCCESS)
    {
        TA_RetCodeInfo info;
        TA_SetRetCodeInfo(rc, &info);
        DEBUG_TRACE("calculate_ma(): %s", info.infoStr);
        return false;
    }

    return (count == ma_count);
}

} // namespace fx

