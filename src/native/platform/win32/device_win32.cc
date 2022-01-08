#include "device_win32.h"

#include <cstdio>
#include <cwchar>

#include <windows.h>
#include <tlhelp32.h>

#include "api.h"
#include "injector_create_remote_thread.h"
#include "console_win32.h"

namespace migi {

Console* DeviceWin32::createConsole()
{
    return new ConsoleWin32();
}

Injector* DeviceWin32::createInjector(uint32_t pid)
{
    return new InjectorCreateRemoteThread(pid);
}

uint32_t DeviceWin32::findProcessByName(const std::string& processName)
{
    Handle hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    CHECK(hProcessSnap, "CreateToolhelp32Snapshot (of processes)");

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    CHECK(Process32First(static_cast<HANDLE>(hProcessSnap), &pe32), "Process32First");

    do
    {
        if(lstrcmpi(processName.c_str(), pe32.szExeFile) == 0)
            return pe32.th32ProcessID;

    } while(Process32Next(static_cast<HANDLE>(hProcessSnap), &pe32));

    return 0;
}

}
