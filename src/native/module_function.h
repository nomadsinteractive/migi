#ifndef MIGI_MODULE_FUNCTION_H_
#define MIGI_MODULE_FUNCTION_H_

#include <utility>
#include <stdint.h>

#include <windows.h>
#include <string>

template<typename T, class R, class... Args> class ModuleFunctionBase {
public:
    typedef T PtrType;

    ModuleFunctionBase(uintptr_t offset)
        : _absolute_addr(true), _entry_addr(offset) {
    }

    ModuleFunctionBase(std::string moduleName, uintptr_t offset)
        : _absolute_addr(false), _entry_addr(offset), _module_name(std::move(moduleName)) {
    }

    R operator ()(Args... args) {
        return (this->ensureEntryPoint())(std::forward<Args>(args)...);
    }

    template<typename U> R invoke(U* self, Args... args) {
        return (self->*(this->ensureEntryPoint()))(std::forward<Args>(args)...);
    }

    PtrType ensureEntryPoint() {
        if(_absolute_addr)
            *(reinterpret_cast<uint32_t*>(&_delegate)) = _entry_addr;
        else {
            HMODULE hMod = GetModuleHandle(_module_name.c_str());
            DWORD dwAddr = reinterpret_cast<DWORD>(hMod) + _entry_addr;
            *(reinterpret_cast<uintptr_t*>(&_delegate)) = dwAddr;
        }
        return _delegate;
    }

private:
    PtrType _delegate;

    bool _absolute_addr;
    uintptr_t _entry_addr;
    std::string _module_name;
};


template <typename R, typename... Args> using CdeclModuleFunction = ModuleFunctionBase<R(__cdecl *)(Args...), R, Args...>;
template <typename R, typename... Args> using StdcallModuleFunction = ModuleFunctionBase<R(__stdcall *)(Args...), R, Args...>;
template <typename R, typename... Args> using FastcallModuleFunction = ModuleFunctionBase<R(__fastcall *)(Args...), R, Args...>;
template <typename T, typename R, typename... Args> using MemberModuleFunction = ModuleFunctionBase<R(T::*)(Args...), R, Args...>;

#endif
