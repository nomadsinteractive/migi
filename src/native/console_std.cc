#include "console_std.h"

#include <iostream>

namespace migi {

void ConsoleStd::show()
{
}

void ConsoleStd::close()
{
}

std::string ConsoleStd::readLine()
{
    std::string line;
    std::cin >> line;
    return line;
}

uint32_t ConsoleStd::write(const std::string& output)
{
    std::cout << output;
    return output.length();
}

}
