#include <string>
#include "logger.h"
#include "config.h"
#include "system_dirs.h"
#include <log4cxx/patternlayout.h>
#include <log4cxx/consoleappender.h>
#include <log4cxx/rollingfileappender.h>
#include <log4cxx/propertyconfigurator.h>

using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace fx::system;

namespace fx {

logger::logger() :
    logger_ptr_(Logger::getRootLogger())
{
    path_type config_path = config::get_log_config();

    if (!config_path.empty() && fs::exists(config_path) && fs::is_regular_file(config_path))
    {
        PropertyConfigurator::configure(config_path.string());
    }
    else
    {
        logger_ptr_->setLevel(Level::getDebug());

        path_type exe_path;
        get_exe_path(exe_path);
        path_type log_name = exe_path.filename().replace_extension(".log");

    #if defined(_WIN32)
        path_type exe_dir = exe_path.parent_path();
        path_type log_path = exe_dir / log_name;

        LayoutPtr layout_ptr(new PatternLayout(L"%d %-5p - %m%n"));
        logger_ptr_->addAppender(new ConsoleAppender(layout_ptr, log_path.wstring()));
        RollingFileAppenderPtr file_appender_ptr(new RollingFileAppender(layout_ptr, log_path.wstring()));
    #else
        path_type log_path = path_type("/var/log") / log_name;

        LayoutPtr layout_ptr(new PatternLayout("%d %-5p - %m%n"));
        logger_ptr_->addAppender(new ConsoleAppender(layout_ptr, log_path.string()));
        RollingFileAppenderPtr file_appender_ptr(new RollingFileAppender(layout_ptr, log_path.string()));
    #endif
        
        file_appender_ptr->setMaximumFileSize(1000000);
        file_appender_ptr->setMaxBackupIndex(2);
        logger_ptr_->addAppender(file_appender_ptr);
    }
}

void logger::warning(const std::string& message)
{
    LOG4CXX_WARN(logger_ptr_, message);
}

void logger::error(const std::string& message)
{
    LOG4CXX_ERROR(logger_ptr_, message);
}

void logger::info(const std::string& message)
{
    LOG4CXX_INFO(logger_ptr_, message);
}

void logger::debug(const std::string& message)
{
    LOG4CXX_DEBUG(logger_ptr_, message);
}
} // namespace fx
