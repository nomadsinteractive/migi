#include "console_win32.h"

#include "migi.h"

namespace migi {

void ConsoleWin32::show()
{
    AllocConsole();
    SetConsoleOutputCP(CP_UTF8);
    CHECK(SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT), "%d", GetLastError());
}

void ConsoleWin32::close()
{
    FreeConsole();
}

std::string ConsoleWin32::readLine()
{
    DWORD dwInputRead = 0;
    char szConsoleBuffer[1024];
    return ReadConsole(GetStdHandle(STD_INPUT_HANDLE), szConsoleBuffer, sizeof(szConsoleBuffer), &dwInputRead, nullptr) ? std::string(szConsoleBuffer, dwInputRead) : "";
}

uint32_t ConsoleWin32::write(const std::string& output)
{
    DWORD dwSize = 0;
    if(!output.empty())
        WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), output.c_str(), output.length(), &dwSize, nullptr);
    return dwSize;
}

}
