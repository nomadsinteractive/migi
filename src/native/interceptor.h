#ifndef MIGI_DETOURED_FUNCTION_H_
#define MIGI_DETOURED_FUNCTION_H_

#include <stdint.h>

#include <memory>
#include <utility>
#include <string>

class Interceptor {
public:
    Interceptor(void* original, void* detouredTo, void** entryPtr = nullptr);
    ~Interceptor();

    int32_t intercept();
    int32_t restore();

    void* functionEntry() const;

    class ScopedTransaction {
    public:
        ScopedTransaction();
        ~ScopedTransaction();
    };

private:
    void* _function_original;
    void* _function_detoured_to;

    void* _function_entry;
    void** _function_entry_ptr;
};

std::shared_ptr<Interceptor> makeInterceptor(void* functionOriginal, void* functionDetouredTo, void** functionEntryPtr);
void clearInterceptors();

template<typename T, typename R, typename... Args> class DetouredFunctionBase {
public:
    typedef T PtrType;

    template<typename U, typename V> DetouredFunctionBase(U wrapper, V funcaddr)
        : _function_detoured_to(*reinterpret_cast<void**>(&wrapper)) {
        *(reinterpret_cast<uint32_t*>(&_function_original_entry)) = toPtr(funcaddr);
    }
    template<typename U> DetouredFunctionBase(U wrapper, uintptr_t funcaddr, std::string moduleName)
        : _function_detoured_to(*reinterpret_cast<void**>(&wrapper)), _function_original_entry(nullptr), _func_addr(funcaddr), _module_name(std::move(moduleName)) {
    }

    void attach() {
        if(_function_original_entry == nullptr)
            *(reinterpret_cast<uint32_t*>(&_function_original_entry)) = getFunctionAddr(_module_name.c_str(), _func_addr);
        std::shared_ptr<Interceptor> attachment = makeInterceptor(reinterpret_cast<void*>(toPtr(_function_original_entry)), _function_detoured_to, reinterpret_cast<void**>(&_function_original_entry));
        const Interceptor::ScopedTransaction transaction;
        attachment->intercept();
    }

    R operator()(Args... args) {
        return _function_original_entry(args...);
    }

    void* _function_detoured_to;
    PtrType _function_original_entry;

    uintptr_t _func_addr;
    std::string _module_name;
};

template <typename R, typename... Args> using DetouredCdeclFunction = DetouredFunctionBase<R(__cdecl *)(Args...), R, Args...>;
template <typename R, typename... Args> using DetouredStdcallFunction = DetouredFunctionBase<R(__stdcall *)(Args...), R, Args...>;
template <typename R, typename... Args> using DetouredFastcallFunction = DetouredFunctionBase<R(__fastcall *)(Args...), R, Args...>;

template<class T, class U> class DetouredClassMethod;

template<class T, class R, class... Args> class DetouredClassMethod<T, R(Args...)> : public DetouredFunctionBase<R (T::*)(Args...), R, Args...> {
public:
    template<typename U> DetouredClassMethod(U wrapper, uint32_t funcAddr, std::string moduleName)
        : DetouredFunctionBase<R (T::*)(Args...), R, Args...>(wrapper, funcAddr, std::move(moduleName)) {
    }

    R invoke(T* thiz, Args... args) {
        return (thiz->*(this->_function_original_entry))(args...);
    }

};

#endif
