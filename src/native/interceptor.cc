#include "interceptor.h"

#include <vector>

#include <windows.h>
#include <detours.h>

static std::vector<std::shared_ptr<Interceptor>> _interceptors;

Interceptor::Interceptor(void* original, void* detouredTo, void** entry)
    : _function_original(original), _function_detoured_to(detouredTo), _function_entry_ptr(entry ? entry : &_function_entry) {
    (*_function_entry_ptr) = _function_original;
}

Interceptor::~Interceptor()
{
    restore();
}

int32_t Interceptor::intercept()
{
    if(*_function_entry_ptr != _function_original)
        return 0;

    return DetourAttach(_function_entry_ptr, _function_detoured_to);
}

int32_t Interceptor::restore()
{
    if(*_function_entry_ptr == _function_original)
        return 0;

    return DetourDetach(_function_entry_ptr, _function_detoured_to);
}

void* Interceptor::functionEntry() const
{
    return _function_entry;
}

Interceptor::ScopedTransaction::ScopedTransaction()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
}

Interceptor::ScopedTransaction::~ScopedTransaction()
{
    DetourTransactionCommit();
}

std::shared_ptr<Interceptor> makeInterceptor(void* original, void* detouredTo, void** functionEntryPtr)
{
    std::shared_ptr<Interceptor> attachment = std::make_shared<Interceptor>(original, detouredTo, functionEntryPtr);
    _interceptors.push_back(attachment);
    return attachment;
}

void clearInterceptors()
{
    const Interceptor::ScopedTransaction transaction;
    for(const std::shared_ptr<Interceptor>& i : _interceptors)
        i->restore();
    _interceptors.clear();
}
