#ifndef MIGI_PLATFORM_WIN32_DEVICE_WIN32_H_
#define MIGI_PLATFORM_WIN32_DEVICE_WIN32_H_

#include "intf/device.h"


namespace migi {

class DeviceWin32 : public Device {
public:

    virtual Console* createConsole() override;
    virtual Injector* createInjector(uint32_t pid) override;
    virtual uint32_t findProcessByName(const std::string& processName) override;
};

}

#endif
