#ifndef _MIGI_PLATFORM_PLATFORM_WIN32_H
#define _MIGI_PLATFORM_PLATFORM_WIN32_H

#include "platform/platform.h"

#include <windows.h>

namespace migi {

class HandleWin32 {
public:
    HandleWin32(HANDLE handle = INVALID_HANDLE_VALUE)
        : _handle(handle) {
    }
    HandleWin32(HandleWin32&& other)
        : _handle(other._handle) {
        other._handle = INVALID_HANDLE_VALUE;
    }
    HandleWin32(const HandleWin32& other) = delete;

    ~HandleWin32() {
        if(bool(*this))
            CloseHandle(_handle);
    }

    explicit operator bool() const {
        return _handle && _handle != INVALID_HANDLE_VALUE;
    }

    explicit operator HANDLE() const {
        return _handle;
    }

    bool operator ==(const HandleWin32& other) const {
        return _handle == other._handle;
    }

    bool operator !=(const HandleWin32& other) const {
        return _handle != other._handle;
    }

private:
    HANDLE _handle;
};

typedef HandleWin32 Handle;

}

#endif
