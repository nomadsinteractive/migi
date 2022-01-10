#include <windows.h>

#include <filesystem>
#include <thread>

#include "migi.h"
#include "platform/platform.h"


static void threadMainEntry(LPVOID lpParameter)
{
    std::filesystem::path modulePath = migi::Platform::getModulePath(reinterpret_cast<void*>(threadMainEntry));
    const std::string migiPathStr = std::filesystem::absolute(modulePath).string();
    const char* argv[] = { migiPathStr.c_str() };
    migi::start(1, argv, reinterpret_cast<uintptr_t>(lpParameter));
    VirtualFree(lpParameter, 0, MEM_RELEASE);
}

__declspec(dllexport) BOOL WINAPI DllMain(HINSTANCE /*hinstDLL*/, DWORD fdwReason, LPVOID /*lpReserved*/);


BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID /*lpReserved*/)
{
    switch(fdwReason)
    {
        case DLL_PROCESS_ATTACH: {
            WCHAR* lpThreadDescription = nullptr;
            GetThreadDescription(GetCurrentThread(), &lpThreadDescription);
            if(lpThreadDescription) {
                WCHAR* lpStrEnd;
                migi::setExtraParameterPtr(static_cast<uintptr_t>(std::wcstoull(lpThreadDescription, &lpStrEnd, 10)));
            }
            std::thread mainThread(threadMainEntry, hinstDLL);
            mainThread.detach();
            break;
        }
        case DLL_PROCESS_DETACH:
            migi::detach();
            break;
        case DLL_THREAD_ATTACH:
            break;
        case DLL_THREAD_DETACH:
            break;
    }
    return TRUE;
}
