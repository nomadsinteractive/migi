#ifndef _MIGI_INJECTOR_H_
#define _MIGI_INJECTOR_H_

#include <filesystem>
#include <stdint.h>

namespace migi {

class Injector {
public:
    virtual ~Injector() = default;

    virtual int32_t loadLibrary(const std::filesystem::path& libraryFilePath, uintptr_t parameter) = 0;

    virtual uintptr_t allocMemory(size_t size) = 0;
    virtual size_t writeMemory(uintptr_t remoteAddress, const void* content, size_t contentSize) = 0;
    virtual void freeMemory(uintptr_t remoteAddress) = 0;

};

}

#endif
