#pragma once

#include <iostream>
#include <fstream>



namespace fx
{
class csv
{
public:
    csv(const std::string& filename, char sep = ';');


    ~csv()
    {
        myfile_.close();
    }

    void add_line(const std::string& line)
    {
        myfile_ << line.c_str() << "\n";
        myfile_.flush();
    }

private:
    std::ofstream myfile_;
};

}//namespace fx