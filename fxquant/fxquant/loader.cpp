#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include "debug.h"
#include "config.h"
#include "mongodb.h"
#include "program_options.h"

namespace fx {

static void print_usage(const std::string& program_name)
{
    std::cout << "USAGE:\n";
    std::cout << program_name << "\n";
    std::cout << "  -c <config_file> - path to the configuration file (required)\n";
    std::cout << "  -h               - print help message and exit\n";
}

int loader(int& argc, char* argv[])
{
    program_options po("c:h");

    if (!po.parse(argc, argv))
    {
        print_usage(argv[0]);
        return 1;
    }

    std::string config_path;

    if (!po.get_option('c', config_path) || config_path.empty() || po.has_option('h'))
    {
        print_usage(argv[0]);
        return 1;
    }

    if (!config::read(config_path))
    {
        std::cerr << "ERROR: config file could not be read: '" << config_path << "'\n";
        return 2;
    }

    bool conn_exists = mongodb::instance().test_connection();

    if (!conn_exists)
    {
        std::cout << "ERROR: database server is not accessible!";
        return 3;
    }
    return 0;
}
} // end of namespace fx
