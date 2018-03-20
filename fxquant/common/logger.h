#pragma once
#include <string>
#include <log4cxx/logger.h>

namespace fx {

class logger // singleton
{
public:
    static logger& instance()
    {
        static logger logger_instance;
        return logger_instance;
    }

    // delete copy and move constructors and assign operators
    logger(logger const&) = delete;
    logger(logger&&) = delete;
    logger& operator=(logger const&) = delete;
    logger& operator=(logger &&) = delete;

public:
    void warning(const std::string& message);
    void error(const std::string& message);
    void info(const std::string& message);
    void debug(const std::string& message);

private:
    logger();

private:
    log4cxx::LoggerPtr logger_ptr_;
};

class log
{
public:
    static void warning(const std::string& message)
    {
        logger::instance().warning(message);
    }

    static void error(const std::string& message)
    {
        logger::instance().error(message);
    }

    static void info(const std::string& message)
    {
        logger::instance().info(message);
    }

    static void debug(const std::string& message)
    {
        logger::instance().debug(message);
    }
};

} // namespace fx
