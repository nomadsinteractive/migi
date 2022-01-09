#include "platform_win32.h"

#include <stdio.h>

#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>

#include "migi.h"

#include "device_win32.h"
#include "injector_create_remote_thread.h"

namespace migi {

static void printError(const TCHAR* msg)
{
    DWORD eNum;
    TCHAR sysMsg[256];
    TCHAR* p;

    eNum = GetLastError( );
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, eNum,  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), sysMsg, 256, nullptr);

    p = sysMsg;
    while( ( *p > 31 ) || ( *p == 9 ) )
    ++p;
    do { *p-- = 0; } while( ( p >= sysMsg ) &&
                          ( ( *p == '.' ) || ( *p < 33 ) ) );
    _tprintf(TEXT("\n  WARNING: %s failed with error %d (%s)"), msg, eNum, sysMsg);
}

void Platform::log(Platform::LogLevel logLevel, const std::string& tag, const std::string& message)
{
    if(isConsoleMode())
    {
        DWORD dwSize = 0;
        HANDLE hCout = GetStdHandle(STD_OUTPUT_HANDLE);
        WriteConsole(hCout, tag.c_str(), tag.length(), &dwSize, nullptr);
        if(tag.size() > 0)
            WriteConsole(hCout, ": ", 2, &dwSize, nullptr);
        WriteConsole(hCout, message.c_str(), message.length(), &dwSize, nullptr);
        WriteConsole(hCout, "\n", 1, &dwSize, nullptr);
    }
    else
        printf("%s: %s\n", tag.c_str(), message.c_str());
}

void* Platform::loadLibrary(const std::string& name, uint32_t flags)
{
    return reinterpret_cast<void*>(flags ? LoadLibraryEx(name.c_str(), nullptr, flags) : LoadLibrary(name.c_str()));
}

void Platform::freeLibrary(void* library)
{
    FreeLibrary(reinterpret_cast<HMODULE>(library));
}

std::string Platform::getModulePath(void* procAddr)
{
    char path[MAX_PATH] = {0};
    HMODULE hm = nullptr;
    if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                          GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                          reinterpret_cast<LPCSTR>(procAddr), &hm) == 0)
    {
        uint32_t ret = GetLastError();
        fprintf(stderr, "GetModuleHandle failed, error = %d\n", ret);
        // Return or however you want to handle an error.
    }
    if (GetModuleFileName(hm, path, sizeof(path)) == 0)
    {
        uint32_t ret = GetLastError();
        fprintf(stderr, "GetModuleFileName failed, error = %d\n", ret);
        // Return or however you want to handle an error.
    }
    return path;
}

void* Platform::getModuleProcAddr(void* moduleAddr, const std::string& procName)
{
    return reinterpret_cast<void*>(GetProcAddress(reinterpret_cast<HMODULE>(moduleAddr), procName.c_str()));
}

Device* Platform::createDevice(Device::DeviceType deviceType)
{
    CHECK(deviceType == Device::DEVICE_TYPE_LOCAL_MACHINE, "Unimplemented");
    return new DeviceWin32();
}

Injector* Platform::createInjector(uint32_t pid)
{
    return new InjectorCreateRemoteThread(pid);
}

void* Platform::getModuleAddr(const std::string& moduleName)
{
    return reinterpret_cast<void*>(GetModuleHandle(moduleName.c_str()));
}

}
