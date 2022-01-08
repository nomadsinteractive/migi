#ifndef MIGI_CONSOLE_H_
#define MIGI_CONSOLE_H_

#include <stdint.h>

#include <string>

namespace migi {

class Console {
public:
    virtual ~Console() = default;

    virtual void show() = 0;
    virtual void close() = 0;

    virtual std::string readLine() = 0;
    virtual uint32_t write(const std::string& output) = 0;
};

}

#endif
