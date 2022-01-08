#ifndef MIGI_CONSOLE_STD_H_
#define MIGI_CONSOLE_STD_H_

#include "intf/console.h"

namespace migi {

class ConsoleStd : public Console {
public:
    virtual void show() override;
    virtual void close() override;
    virtual std::string readLine() override;
    virtual uint32_t write(const std::string& output) override;
};

}

#endif
