#include <stdint.h>
#include <memory>
#include <vector>

#include "migi.h"
#include "intf/injector.h"

#include "platform/platform.h"

int32_t main(int32_t argc, const char* argv[])
{
    if(argc <= 1)
        return 1;

    const std::string cmd = argv[1];

    if(argc > 3 && cmd == "inject")
    {
        const std::unique_ptr<migi::Device> device(migi::Platform::createDevice(migi::Device::DEVICE_TYPE_LOCAL_MACHINE));
        uint32_t pid = device->findProcessByName(argv[2]);
        if(pid)
        {
            std::unique_ptr<migi::Injector> injector(device->createInjector(pid));
            for(int32_t i = 3; i < argc; ++i)
                injector->loadLibrary(std::filesystem::absolute(argv[i]), 0);
        }
    }


    if(cmd == "start")
    {
        std::vector<const char*> vArgs;
        for(int32_t i = 0; i < argc; ++i)
            if(i != 1)
                vArgs.push_back(argv[i]);
        start(argc - 1, vArgs.data(), 0);
    }
    return 0;
}
