#include <sstream>
#include "debug.h"
#include "config.h"
#include "mongodb.h"

namespace fx {
void mongodb::logger::operator()(mongocxx::log_level level,
    mongocxx::stdx::string_view domain,
    mongocxx::stdx::string_view message) noexcept
{
    write(level, message.to_string());
}

void mongodb::logger::write(mongocxx::log_level level, const std::string& message)
{
    using namespace mongocxx;
    char level_code = '?';

    switch (level)
    {
    case log_level::k_error:    level_code = 'E'; break;
    case log_level::k_critical: level_code = 'C'; break;
    case log_level::k_warning:  level_code = 'W'; break;
    case log_level::k_message:  level_code = 'M'; break;
    case log_level::k_info:     level_code = 'I'; break;
    case log_level::k_debug:    level_code = 'D'; break;
    case log_level::k_trace:    level_code = 'T'; break;
    }

    ALWAYS_TRACE("mongocxx [%c] %s", level_code, message.c_str()); // works only in Windows
}

mongodb::mongodb(size_t min_pool_size, size_t max_pool_size, unsigned int timeout_ms)
{
    // http://mongodb.github.io/mongo-cxx-driver/mongocxx-v3/configuration/

    // The mongocxx::instance constructor and destructor initialize and shut down the driver,
    // respectively. Therefore, a mongocxx::instance must be created before using the driver and
    // must remain alive for as long as the driver is in use.
    static mongocxx::instance instance_{ std::make_unique<mongodb::logger>() };

    const std::string address = config::get_mongo_ip();
    const unsigned short port = config::get_mongo_port();
    const std::string user = config::get_mongo_user();
    const std::string password = config::get_mongo_password();

    std::stringstream ss;
    ss << "mongodb://";

    if (!user.empty() && !password.empty())
    {
        ss << user << ":" << password << "@";
    }

    ss << address << ":" << port;
    ss << "/?minPoolSize=" << min_pool_size << "&maxPoolSize=" << max_pool_size;
    ss << "&serverSelectionTimeoutMS=" << timeout_ms;
    //ss << "&authSource=admin&authMechanism=MONGODB-CR";

    mongocxx::uri uri{ ss.str() };
    pool_ptr_ = std::make_unique<mongocxx::pool>(uri);
}

bool mongodb::enumerate_databases(std::vector<std::string>& databases)
{
    bool success = false;
    std::vector<std::string> v;

    try
    {
        auto client = pool_ptr_->acquire();

        if (client)
        {
            for (auto doc : client->list_databases())
            {
                v.push_back(doc["name"].get_utf8().value.to_string());
            }

            success = true;
        }
    }
    catch (const std::exception& x)
    {
        DEBUG_TRACE("enumerate_databases(): %s", x.what());
    }

    databases.swap(v);
    return success;
}

bool mongodb::test_connection()
{
    try
    {
        auto client = pool_ptr_->acquire();

        if (client)
        {
            // trying to get a list of databases
            auto doc = client->list_databases().begin();
            return true;
        }
    }
    catch (const std::exception& x)
    {
        DEBUG_TRACE("%s", x.what());
    }

    return false;
}
}//end of namespace fx
