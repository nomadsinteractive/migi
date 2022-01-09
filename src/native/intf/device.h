#ifndef MIGI_DEVICE_H_
#define MIGI_DEVICE_H_

#include <stdint.h>
#include <string>

#include "forwarding.h"

namespace migi {

class Device {
public:
    enum DeviceType {
        DEVICE_TYPE_LOCAL_MACHINE = 0,
    };

    enum ProcessArchitecture {
        PROCESS_ARCHITECTURE_UNKNOWN = 0,
        PROCESS_ARCHITECTURE_ABI_32 = 1,
        PROCESS_ARCHITECTURE_ABI_64 = 2,
        PROCESS_ARCHITECTURE_WOW_64 = 3,
    };

public:
    virtual ~Device() = default;

    virtual Console* createConsole() = 0;
    virtual Injector* createInjector(uint32_t pid) = 0;
    virtual ProcessArchitecture getProcessArchitecture(uint32_t pid) = 0;

    virtual int32_t findProcessByName(const std::string& processName) = 0;
};

}

#endif
