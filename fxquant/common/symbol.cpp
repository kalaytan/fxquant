#include <map>
#include <iomanip>
#include <algorithm>
#include "symbol.h"

namespace fx {

std::string symbol_to_string(symbol sym, bool uppercase)
{
    static const std::map<symbol, std::string> sym_to_str =
    {
        { symbol::audcad, "audcad" },
        { symbol::audchf, "audchf" },
        { symbol::audhuf, "audhuf" },
        { symbol::audjpy, "audjpy" },
        { symbol::audnok, "audnok" },
        { symbol::audnzd, "audnzd" },
        { symbol::audsek, "audsek" },
        { symbol::audsgd, "audsgd" },
        { symbol::audusd, "audusd" },
        { symbol::cadchf, "cadchf" },
        { symbol::cadjpy, "cadjpy" },
        { symbol::chfjpy, "chfjpy" },
        { symbol::chfdkk, "chfdkk" },
        { symbol::chfhuk, "chfhuk" },
        { symbol::chfnok, "chfnok" },
        { symbol::chfpln, "chfpln" },
        { symbol::chfsek, "chfsek" },
        { symbol::euraud, "euraud" },
        { symbol::eurcad, "eurcad" },
        { symbol::eurchf, "eurchf" },
        { symbol::eurczk, "eurczk" },
        { symbol::eurdkk, "eurdkk" },
        { symbol::eurhkd, "eurhkd" },
        { symbol::eurhuf, "eurhuf" },
        { symbol::eurgbp, "eurgbp" },
        { symbol::eurjpy, "eurjpy" },
        { symbol::eurils, "eurils" },
        { symbol::eurmxn, "eurmxn" },
        { symbol::eurnok, "eurnok" },
        { symbol::eurnzd, "eurnzd" },
        { symbol::eurpln, "eurpln" },
        { symbol::eursek, "eursek" },
        { symbol::eursgd, "eursgd" },
        { symbol::eurtry, "eurtry" },
        { symbol::eurusd, "eurusd" },
        { symbol::eurzar, "eurzar" },
        { symbol::gbpaud, "gbpaud" },
        { symbol::gbpcad, "gbpcad" },
        { symbol::gbpchf, "gbpchf" },
        { symbol::gbpdkk, "gbpdkk" },
        { symbol::gbpjpy, "gbpjpy" },
        { symbol::gbphuf, "gbphuf" },
        { symbol::gbpmxn, "gbpmxn" },
        { symbol::gbpnok, "gbpnok" },
        { symbol::gbpnzd, "gbpnzd" },
        { symbol::gbppln, "gbppln" },
        { symbol::gbpsek, "gbpsek" },
        { symbol::gbpsgd, "gbpsgd" },
        { symbol::gbptry, "gbptry" },
        { symbol::gbpusd, "gbpusd" },
        { symbol::gbpzar, "gbpzar" },
        { symbol::nokjpy, "nokjpy" },
        { symbol::nzdcad, "nzdcad" },
        { symbol::nzdchf, "nzdchf" },
        { symbol::nzdhuf, "nzdhuf" },
        { symbol::nzdjpy, "nzdjpy" },
        { symbol::nzdsgd, "nzdsgd" },
        { symbol::nzdusd, "nzdusd" },
        { symbol::usdcad, "usdcad" },
        { symbol::usdchf, "usdchf" },
        { symbol::usdcnh, "usdcnh" },
        { symbol::usdczk, "usdczk" },
        { symbol::usddkk, "usddkk" },
        { symbol::usdhkd, "usdhkd" },
        { symbol::usdhuf, "usdhuf" },
        { symbol::usdils, "usdils" },
        { symbol::usdjpy, "usdjpy" },
        { symbol::usdmxn, "usdmxn" },
        { symbol::usdnok, "usdnok" },
        { symbol::usdpln, "usdpln" },
        { symbol::usdrub, "usdrub" },
        { symbol::usdsek, "usdsek" },
        { symbol::usdsgd, "usdsgd" },
        { symbol::usdtry, "usdtry" },
        { symbol::usdzar, "usdzar" }
    };

    auto it = sym_to_str.find(sym);

    if (it == sym_to_str.end())
    {
        return "";
    }

    if (!uppercase)
    {
        return it->second;
    }

    std::string s(it->second);
    std::transform(s.begin(), s.end(), s.begin(), ::toupper);
    return s;
}

symbol symbol_from_string(const std::string& str)
{
    static const std::map<std::string, symbol> str_to_sym =
    {
        { "audcad", symbol::audcad },
        { "audchf", symbol::audchf },
        { "audhuf", symbol::audhuf },
        { "audjpy", symbol::audjpy },
        { "audnok", symbol::audnok },
        { "audnzd", symbol::audnzd },
        { "audsek", symbol::audsek },
        { "audsgd", symbol::audsgd },
        { "audusd", symbol::audusd },
        { "cadchf", symbol::cadchf },
        { "cadjpy", symbol::cadjpy },
        { "chfjpy", symbol::chfjpy },
        { "chfdkk", symbol::chfdkk },
        { "chfhuk", symbol::chfhuk },
        { "chfnok", symbol::chfnok },
        { "chfpln", symbol::chfpln },
        { "chfsek", symbol::chfsek },
        { "euraud", symbol::euraud },
        { "eurcad", symbol::eurcad },
        { "eurchf", symbol::eurchf },
        { "eurczk", symbol::eurczk },
        { "eurdkk", symbol::eurdkk },
        { "eurhkd", symbol::eurhkd },
        { "eurhuf", symbol::eurhuf },
        { "eurgbp", symbol::eurgbp },
        { "eurjpy", symbol::eurjpy },
        { "eurils", symbol::eurils },
        { "eurmxn", symbol::eurmxn },
        { "eurnok", symbol::eurnok },
        { "eurnzd", symbol::eurnzd },
        { "eurpln", symbol::eurpln },
        { "eursek", symbol::eursek },
        { "eursgd", symbol::eursgd },
        { "eurtry", symbol::eurtry },
        { "eurusd", symbol::eurusd },
        { "eurzar", symbol::eurzar },
        { "gbpaud", symbol::gbpaud },
        { "gbpcad", symbol::gbpcad },
        { "gbpchf", symbol::gbpchf },
        { "gbpdkk", symbol::gbpdkk },
        { "gbpjpy", symbol::gbpjpy },
        { "gbphuf", symbol::gbphuf },
        { "gbpmxn", symbol::gbpmxn },
        { "gbpnok", symbol::gbpnok },
        { "gbpnzd", symbol::gbpnzd },
        { "gbppln", symbol::gbppln },
        { "gbpsek", symbol::gbpsek },
        { "gbpsgd", symbol::gbpsgd },
        { "gbptry", symbol::gbptry },
        { "gbpusd", symbol::gbpusd },
        { "gbpzar", symbol::gbpzar },
        { "nokjpy", symbol::nokjpy },
        { "nzdcad", symbol::nzdcad },
        { "nzdchf", symbol::nzdchf },
        { "nzdhuf", symbol::nzdhuf },
        { "nzdjpy", symbol::nzdjpy },
        { "nzdsgd", symbol::nzdsgd },
        { "nzdusd", symbol::nzdusd },
        { "usdcad", symbol::usdcad },
        { "usdchf", symbol::usdchf },
        { "usdcnh", symbol::usdcnh },
        { "usdczk", symbol::usdczk },
        { "usddkk", symbol::usddkk },
        { "usdhkd", symbol::usdhkd },
        { "usdhuf", symbol::usdhuf },
        { "usdils", symbol::usdils },
        { "usdjpy", symbol::usdjpy },
        { "usdmxn", symbol::usdmxn },
        { "usdnok", symbol::usdnok },
        { "usdpln", symbol::usdpln },
        { "usdrub", symbol::usdrub },
        { "usdsek", symbol::usdsek },
        { "usdsgd", symbol::usdsgd },
        { "usdtry", symbol::usdtry },
        { "usdzar", symbol::usdzar }
    };

    std::string s(str);
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    auto it = str_to_sym.find(s);

    if (it == str_to_sym.end())
    {
        return symbol::undefined;
    }

    return it->second;
}

double symbol_pip(symbol sym)
{
    switch (sym)
    {
    case symbol::usdjpy:
    case symbol::eurjpy:
    case symbol::gbpjpy:
    case symbol::chfjpy:
    case symbol::audjpy:
    case symbol::cadjpy:
    case symbol::nokjpy:
    case symbol::nzdjpy:
        return 0.001;
    }

    return 0.00001;
}

} // namespace fx
