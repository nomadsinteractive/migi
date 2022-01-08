#include "injector_create_remote_thread.h"

#include <cstdio>
#include <cwchar>

#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>
#define PSAPI_VERSION 1
#include <psapi.h>

namespace migi {

int PrintModules( DWORD processID )
{
    HMODULE hMods[1024];
    HANDLE hProcess;
    DWORD cbNeeded;
    unsigned int i;


    printf( "\nProcess ID: %u\n", processID );

    hProcess = OpenProcess( PROCESS_QUERY_INFORMATION |
                            PROCESS_VM_READ,
                            FALSE, processID );
    if (hProcess == nullptr)
        return 1;

   // Get a list of all the modules in this process.

    if( EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded))
    {
        for ( i = 0; i < (cbNeeded / sizeof(HMODULE)); i++ )
        {
            TCHAR szModName[MAX_PATH];

            // Get the full path to the module's file.

            if ( GetModuleFileNameEx( hProcess, hMods[i], szModName,
                                      sizeof(szModName) / sizeof(TCHAR)))
            {
                // Print the module name and handle value.

                _tprintf( TEXT("\t%s (0x%p)\n"), szModName, hMods[i] );
            }
        }
    }

    // Release the handle to the process.

    CloseHandle( hProcess );

    return 0;
}

InjectorCreateRemoteThread::InjectorCreateRemoteThread(uint32_t pid)
    : _remote_process(OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_WRITE, FALSE, pid)) {
}

int32_t InjectorCreateRemoteThread::loadLibrary(const std::filesystem::path& libraryFilePath, uintptr_t parameter)
{
    if(!_remote_process)
        return 1;

    const std::string libFile = libraryFilePath.string();
    size_t dwSize = libFile.length() + 1;

    uintptr_t pszLibFileRemote = allocMemory(dwSize);
    if(pszLibFileRemote == 0)
        return 1;

    if(writeMemory(pszLibFileRemote, reinterpret_cast<LPCVOID>(libFile.c_str()), dwSize) != dwSize)
        return 1;

    PTHREAD_START_ROUTINE pfnThreadRtn = reinterpret_cast<PTHREAD_START_ROUTINE>(GetProcAddress(GetModuleHandle(TEXT("Kernel32")), "LoadLibraryA"));
    if (pfnThreadRtn == nullptr)
        return 1;

    Handle hThread = CreateRemoteThread(static_cast<HANDLE>(_remote_process), nullptr, 0, pfnThreadRtn, reinterpret_cast<LPVOID>(pszLibFileRemote), CREATE_SUSPENDED, nullptr);
    if(!hThread)
        return 1;

    wchar_t sDescription[64];
    std::swprintf(sDescription, sizeof(sDescription) / sizeof(sDescription[0]), L"%zu", parameter);

    HRESULT hr = SetThreadDescription(static_cast<HANDLE>(hThread), sDescription);
    if(FAILED(hr))
        return 1;

    uint32_t sh = ResumeThread(static_cast<HANDLE>(hThread));
    if(sh == static_cast<uint32_t>(-1))
        return 1;

    WaitForSingleObject(static_cast<HANDLE>(hThread), INFINITE);

    freeMemory(pszLibFileRemote);
    return 0;
}

uintptr_t InjectorCreateRemoteThread::allocMemory(size_t size)
{
    return reinterpret_cast<uintptr_t>(VirtualAllocEx(static_cast<HANDLE>(_remote_process), nullptr, size, MEM_COMMIT, PAGE_READWRITE));
}

size_t InjectorCreateRemoteThread::writeMemory(uintptr_t remoteAddress, const void* content, size_t contentSize)
{
    SIZE_T numberOfBytesWritten;
    return WriteProcessMemory(static_cast<HANDLE>(_remote_process), reinterpret_cast<LPVOID>(remoteAddress), content, contentSize, &numberOfBytesWritten) ? numberOfBytesWritten : 0;
}

void InjectorCreateRemoteThread::freeMemory(uintptr_t remoteAddress)
{
    VirtualFreeEx(static_cast<HANDLE>(_remote_process), reinterpret_cast<LPVOID>(remoteAddress), 0, MEM_RELEASE);
}

}
