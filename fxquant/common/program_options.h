#pragma once
#include <map>
#include <string>

class program_options
{
public:
    program_options(const std::string& opt_string);

    bool parse(int argc, char* argv[]);
    bool has_option(char c) const;
    bool get_option(char c, std::string& opt) const;

private:
    const std::string opt_string_;
    std::map<char, std::string> options_;
};