#include <getopt.h>
#include "program_options.h"

program_options::program_options(const std::string& opt_string) :
    opt_string_(opt_string)
{
}

bool program_options::parse(int argc, char* argv[])
{
    int opt = 0;
    options_.clear();

    while ((opt = getopt(argc, argv, opt_string_.c_str())) != -1)
    {
        switch (opt)
        {
        case '?':
            return false; // unrecognized option
        case ':':
            return false; // missing argument
        default:
            if (optarg)
            {
                options_.insert({ opt, optarg });
            }
            else
            {
                options_.insert({ opt, "" });
            }
        }
    }

    return true;
}

bool program_options::has_option(char c) const
{
    return options_.find(c) != options_.end();
}

bool program_options::get_option(char c, std::string& opt) const
{
    opt.clear();
    auto it = options_.find(c);

    if (it != options_.end())
    {
        opt = it->second;
        return true;
    }

    return false;
}
