#include "csv.h"
#include "config.h"
#include <ctime>
#include <filesystem>

namespace fs = std::experimental::filesystem;
namespace {
bool create_dir(const fs::path& dir)
{
    try
    {
        if (!fs::exists(dir))
        {
            return fs::create_directories(dir);
        }
        return true;
    }
    catch (const std::exception&)
    {
        return false;
    }
}
const std::string now_to_string()
{
    char time_str[20];
    time_t time = std::time(0);
    strftime(time_str, sizeof(time_str), "%Y%m%d%H%M%S", gmtime(&time));
    return time_str;
}
}//end of namespace

namespace fx
{
csv::csv(const std::string& subdir, char sep)
{
    auto csv_dir = config::instance().get_csv_dir() / subdir;
    if (!create_dir(csv_dir))
    {
        throw std::runtime_error("Failed to create csv directory.");
    }

    fs::path csv_path = csv_dir / (now_to_string() + ".csv");
    myfile_.open(csv_path);

    if (!myfile_.is_open())
    {
        throw std::runtime_error("Failed to open csv file.");
    }
    myfile_ << "sep=" << sep << "\n";
}

}//end of namespace fx