#ifndef MIGI_PLATFORM_H_
#define MIGI_PLATFORM_H_

#include <stdint.h>
#include <string>

#include "forwarding.h"
#include "intf/device.h"

namespace migi {

class Platform {
public:
    enum LogLevel {
        LOG_LEVEL_INFO = 0,
        LOG_LEVEL_DEBUG = 1,
        LOG_LEVEL_WARNING = 2,
        LOG_LEVEL_ERROR = 3
    };

    static void log(LogLevel logLevel, const std::string& tag, const std::string& message);

    static void* loadLibrary(const std::string& name, uint32_t flags);
    static void freeLibrary(void* library);

    static std::string getModulePath(void* procAddr);

    static void* getModuleAddr(const std::string& moduleName);
    static void* getModuleProcAddr(void* moduleAddr, const std::string& procName);

    static Device* createDevice(Device::DeviceType deviceType);
    static Injector* createInjector(uint32_t pid);

};

}

#endif
