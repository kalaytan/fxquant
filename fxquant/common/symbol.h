#pragma once
#include <string>

namespace fx {

enum class symbol
{
    undefined = 0,
    audcad, audchf, audhuf, audjpy, audnok, audnzd, audsek, audsgd, audusd,
    cadchf, cadjpy,
    chfjpy, chfdkk, chfhuk, chfnok, chfpln, chfsek,
    euraud, eurcad, eurchf, eurczk, eurdkk, eurhkd, eurhuf, eurgbp, eurjpy, eurils,
    eurmxn, eurnok, eurnzd, eurpln, eursek, eursgd, eurtry, eurusd, eurzar,
    gbpaud, gbpcad, gbpchf, gbpdkk, gbpjpy, gbphuf, gbpmxn, gbpnok, gbpnzd, gbppln,
    gbpsek, gbpsgd, gbptry, gbpusd, gbpzar,
    nokjpy,
    nzdcad, nzdchf, nzdhuf, nzdjpy, nzdsgd, nzdusd,
    usdcad, usdchf, usdcnh, usdczk, usddkk, usdhkd, usdhuf, usdils, usdjpy, usdmxn,
    usdnok, usdpln, usdrub, usdsek, usdsgd, usdtry, usdzar
};

std::string symbol_to_string(symbol sym, bool uppercase = false);
symbol symbol_from_string(const std::string& str);
double symbol_pip(symbol sym);

} // namespace fx
