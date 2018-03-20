#pragma once
#include <string>
#include <vector>
#include <memory>
#include <mongocxx/uri.hpp>
#include <mongocxx/pool.hpp>
#include <mongocxx/logger.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>

namespace fx {
class mongodb // singleton
{
public:
    static mongodb& instance()
    {
        static mongodb mongodb_instance(1, 20, 4000);
        return mongodb_instance;
    }

    // delete copy and move constructors and assign operators
    mongodb(mongodb const&) = delete;
    mongodb(mongodb&&) = delete;
    mongodb& operator=(mongodb const&) = delete;
    mongodb& operator=(mongodb &&) = delete;

public:
    struct logger final : public mongocxx::logger
    {
        void operator()(mongocxx::log_level level,
            mongocxx::stdx::string_view domain,
            mongocxx::stdx::string_view message) noexcept override;

        void write(mongocxx::log_level level, const std::string& message);
    };

    typedef mongocxx::v_noabi::pool::entry client_type;

public:
    // Returns 'true' if database server is accessible, or 'false' otherwise
    bool test_connection();

    // Fills the vector with the names of available databases
    bool enumerate_databases(std::vector<std::string>& databases);

    // Acquires a client from the pool.
    // The calling thread will block until a connection is available.
    client_type get_client()
    {
        return pool_ptr_->acquire();
    }

private:
    explicit mongodb(size_t min_pool_size = 1, size_t max_pool_size = 1,
        unsigned int timeout_ms = 4000);

private:
    std::unique_ptr<mongocxx::pool> pool_ptr_;
};

typedef std::shared_ptr<mongodb> mongodb_ptr;
}//end of namespace fx
