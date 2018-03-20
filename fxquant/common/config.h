#pragma once
#include <string>
#include <json/value.h>
#include <filesystem>

namespace fx {

class config // singleton
{
public:
    static config& instance()
    {
        // C++11 standard (6.7.4):
        // If control enters the declaration concurrently while the variable is
        // being initialized, the concurrent execution shall wait for completion
        // of the initialization.
        //
        static config config_instance;
        return config_instance;
    }

    // delete copy and move constructors and assign operators
    config(config const&) = delete;
    config(config&&) = delete;
    config& operator=(config const&) = delete;
    config& operator=(config &&) = delete;

    static bool read(const std::string& config_path)
    {
        return instance().read_internal(config_path);
    }

    static std::string get_mongo_ip()
    {
        return instance().read_string("mongodb", "mongoip", "127.0.0.1");
    }

    static uint16_t get_mongo_port()
    {
        return static_cast<uint16_t>(instance().read_int("mongodb", "mongoport", 27017));
    }

    static std::string get_mongo_user()
    {
        return instance().read_string("mongodb", "mongouser", "");
    }

    static std::string get_mongo_password()
    {
        return instance().read_string("mongodb", "mongopass", "");
    }

    static uint16_t get_server_port()
    {
        return static_cast<uint16_t>(instance().read_int("server", "port", 65432));
    }

    static std::string get_log_config()
    {
        return instance().read_string("logger", "config", "log4cxx.cfg");
    }

    static std::string get_log_name()
    {
        return instance().read_string("logger", "name", "collector");
    }
    static std::experimental::filesystem::path get_csv_dir()
    {
        return instance().read_string("csv", "dir", "reports");
    }

    static std::string get_string(const std::string& section, const std::string& param)
    {
        //  check if not found and throw runtime error
        return instance().read_string(section, param);
    }

    static int get_int(const std::string& section, const std::string& param)
    {
        return instance().read_int(section, param);
    }

    static double get_double(const std::string& section, const std::string& param)
    {
        return instance().read_double(section, param);
    }

    static bool get_bool(const std::string& section, const std::string& param = false)
    {
        //todo check if not found and throw runtime error
        return instance().read_bool(section, param);
    }

    int read_int(const std::string& section, const std::string& param,
        int default_value) const;
    int read_int(const std::string& section, const std::string& param) const;

    double read_double(const std::string& section, const std::string& param,
        int default_value) const;
    double read_double(const std::string& section, const std::string& param) const;

    std::string read_string(const std::string& section, const std::string& param,
        const std::string& default_value) const;
    std::string read_string(const std::string& section, const std::string& param) const;

    bool read_bool(const std::string& section, const std::string& param, const bool& default_value);
    bool read_bool(const std::string& section, const std::string& param);

    bool has_param(const std::string& section, const std::string& param) const;

private:
    config() = default;
    bool read_internal(const std::string& config_path);

private:
    Json::Value root_;
};
} // end of namespace fx
