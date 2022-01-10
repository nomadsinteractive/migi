#ifndef MIGI_API_H_
#define MIGI_API_H_

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "forwarding.h"

#if INTPTR_MAX == INT32_MAX
    #define MIGI_ARCH_32
#elif INTPTR_MAX == INT64_MAX
    #define MIGI_ARCH_64
#else
    #error "Environment not 32 or 64-bit."
#endif

#if defined(__clang__)
#   define __MIGI_FUNCTION__     __PRETTY_FUNCTION__
#elif defined(__GNUC__) || defined(__GNUG__)
#   define __MIGI_FUNCTION__     __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
#   define __MIGI_FUNCTION__     __FUNCTION__
#endif

#define FATAL(...) migi::__fatal__(__MIGI_FUNCTION__, nullptr, __VA_ARGS__)
#define CHECK(cond, ...) if(!(cond)) migi::__fatal__(__MIGI_FUNCTION__, #cond, __VA_ARGS__)
#define WARN(cond, ...) if(!(cond)) migi::__warning__(__MIGI_FUNCTION__, __VA_ARGS__)

#ifdef MIGI_FLAG_DEBUG
#   define DFATAL(...) FATAL(__VA_ARGS__)
#   define DCHECK(cond, ...) CHECK(cond, __VA_ARGS__)
#   define DWARN(cond, ...) WARN(cond, __VA_ARGS__)
#   define DTRACE(cond) if(cond) __trace__()
#   define DTHREAD_CHECK(threadId) __thread_check__<threadId>(__MIGI_FUNCTION__)
#else
#   define DFATAL(...)
#   define DCHECK(cond, ...) (void (cond))
#   define DWARN(cond, ...) (void (cond))
#   define DTRACE(cond) (void (cond))
#   define DTHREAD_CHECK(threadId) (void (threadId))
#endif

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
    TypeName(const TypeName&) = delete;   \
    TypeName& operator=(const TypeName&) = delete

#define DEFAULT_COPY_AND_ASSIGN(TypeName) \
    TypeName(const TypeName&) = default;   \
    TypeName(TypeName&&) = default;   \
    TypeName& operator=(const TypeName&) = default;   \
    TypeName& operator=(TypeName&&) = default

#define DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(TypeName) \
    TypeName(const TypeName&) noexcept = default;   \
    TypeName(TypeName&&) noexcept = default;   \
    TypeName& operator=(const TypeName&) noexcept = default;   \
    TypeName& operator=(TypeName&&) noexcept = default

namespace migi {

[[noreturn]]
void __fatal__(const char* func, const char* condition, const char* format, ...);
void __warning__(const char* func, const char* format, ...);

bool is_mocked();
uint32_t sizeof_void_p();
void sleep_for(float seconds);
std::shared_ptr<Device> create_device(int32_t deviceType);
std::string dump_bytes(const std::string& bytes);

uintptr_t load_library(const std::string& libraryName, uint32_t flags);
void free_library(uintptr_t library);

uint64_t make_call(uintptr_t procAddr, const std::vector<uintptr_t>& args, uint32_t callStackSize);
uint64_t make_thiscall(uintptr_t procAddr, uintptr_t thisPtr, const std::vector<uintptr_t>& args, uint32_t callStackSize);
uint64_t make_fastcall(uintptr_t procAddr, uintptr_t arg0, uintptr_t arg1, const std::vector<uintptr_t>& args, uint32_t callStackSize);

uintptr_t stdcall_to_fastcall(uintptr_t stdcallFunctionPtr, bool ensureGIL, uintptr_t preCallPtr, uintptr_t postCallPtr);
void stdcall_to_fastcall_recycle(uintptr_t fastcallFunctionPtr);

uintptr_t stdcall_to_thiscall(uintptr_t stdcallFunctionPtr, bool ensureGIL, uintptr_t preCallPtr, uintptr_t postCallPtr);
void stdcall_to_thiscall_recycle(uintptr_t fastcallFunctionPtr);

std::shared_ptr<Interceptor> make_interceptor(uintptr_t originalFunc, uintptr_t detoureFunc);

void start_console(std::vector<std::string> commands);

const std::string& get_module_file_path();

void logd(const std::string& message);
uintptr_t get_module_address(const std::string& moduleName, uintptr_t offset);
uintptr_t get_module_proc(const std::string& moduleName, const std::string& procName);

const std::map<std::string, std::string>& get_properties();

}

#endif
