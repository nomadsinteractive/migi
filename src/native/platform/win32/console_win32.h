#ifndef MIGI_PLATFORM_WIN32_CONSOLE_WIN32_H_
#define MIGI_PLATFORM_WIN32_CONSOLE_WIN32_H_

#include "intf/console.h"

#include <windows.h>

namespace migi {

class ConsoleWin32 : public Console {
public:

    virtual void show() override;
    virtual void close() override;

    virtual std::string readLine() override;
    virtual uint32_t write(const std::string& output) override;


private:
    HANDLE _input;
    HANDLE _output;
};

}

#endif
