#ifndef MIGI_INJECTOR_CREATE_REMOTE_THREAD_H_
#define MIGI_INJECTOR_CREATE_REMOTE_THREAD_H_

#include "intf/injector.h"

#include "platform_win32.h"

namespace migi {

class InjectorCreateRemoteThread : public Injector {
public:
    InjectorCreateRemoteThread(uint32_t pid);

    virtual int32_t loadLibrary(const std::filesystem::path& libraryFilePath, uintptr_t parameter) override;

    virtual uintptr_t allocMemory(size_t size) override;
    virtual size_t writeMemory(uintptr_t remoteAddress, const void* content, size_t contentSize) override;
    virtual void freeMemory(uintptr_t remoteAddress) override;

private:
    Handle _remote_process;
};

}

#endif
