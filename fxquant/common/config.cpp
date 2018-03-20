#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <json/reader.h>

#include "debug.h"
#include "config.h"
#include "mongodb.h"

namespace fx {
bool config::read_internal(const std::string& config_path)
{
    DEBUG_ASSERT(!config_path.empty());
    // http://open-source-parsers.github.io/jsoncpp-docs/doxygen/class_json_1_1_char_reader_builder.html

    std::fstream file;
    file.open(config_path);

    if (!file.is_open())
    {
        DEBUG_TRACE("Config file does not exist: '%s'", config_path.c_str());
        return false;
    }

    Json::Reader reader;
    Json::CharReaderBuilder rbuilder;
    rbuilder["collectComments"] = false;

    std::string errs;
    bool ok = Json::parseFromStream(rbuilder, file, &root_, &errs);
    file.close();

    if (ok)
    {
        return true;
    }

    DEBUG_TRACE("Failed to parse the configuration file");
    return false;
}

int config::read_int(const std::string& section, const std::string& param,
    int default_value) const
{
    return root_[section].get(param, default_value).asInt();
}
int config::read_int(const std::string& section, const std::string& param) const
{
    try
    {
        if (has_param(section, param) && root_[section][param].isInt())
        {
            return root_[section].get(param, -1).asInt();
        }
    }
    catch (...)
    {
    }
    throw std::runtime_error("config::read_int() failed.");
}

double config::read_double(const std::string& section, const std::string& param,
    int default_value) const
{
    return root_[section].get(param, default_value).asDouble();
}
double config::read_double(const std::string& section, const std::string& param) const
{
    try
    {
        if (has_param(section, param) && root_[section][param].isDouble())
        {
            return root_[section].get(param, -1).asDouble();
        }
    }
    catch (...)
    {
    }
    throw std::runtime_error("config::read_double() failed.");
}

std::string config::read_string(const std::string& section, const std::string& param,
    const std::string& default_value)  const
{
    return root_[section].get(param, default_value).asString();
}

std::string config::read_string(const std::string& section, const std::string& param) const
{
    try
    {
        if (has_param(section, param) && root_[section][param].isString())
        {
            return root_[section].get(param, "").asString();
        }
    }
    catch (...)
    {
    }
    throw std::runtime_error("config::read_string() failed.");
}

bool config::read_bool(const std::string& section, const std::string& param, const bool& default_value)
{
    return root_[section].get(param, default_value).asBool();
}

bool config::read_bool(const std::string& section, const std::string& param)
{
    try
    {
        if (has_param(section, param) && root_[section][param].isBool())
        {
            return root_[section].get(param, false).asBool();
        }
    }
    catch (...)
    {
    }
    throw std::runtime_error("config::read_bool() failed.");
}

bool config::has_param(const std::string& section, const std::string& param) const
{
    return root_[section].isMember(param);
}

} // end of namespace fx
