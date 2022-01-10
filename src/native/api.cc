#include "migi.h"

#include <array>
#include <cstdarg>
#include <thread>

#include "intf/device.h"
#include "platform/platform.h"

#include "interceptor.h"

#include "py/python_api.h"

#ifdef MIGI_ARCH_32
#include "generated/stdcall_to_fastcall_function_table.h"
#endif
#include "generated/stdcall_to_thiscall_function_table.h"


namespace migi {

extern std::map<std::string, std::string> gProperties;

void __fatal__(const char* func, const char* condition, const char* format, ...)
{
    char buf[2048];
    if(condition)
    {
        std::snprintf(buf, sizeof(buf), "\"%s\" failed! ", condition);
        migi::Platform::log(migi::Platform::LOG_LEVEL_ERROR, func, buf);
    }

    va_list args;
    va_start(args, format);
    std::vsnprintf(buf, sizeof(buf), format, args);
    migi::Platform::log(migi::Platform::LOG_LEVEL_ERROR, func, buf);
    va_end(args);
    throw std::logic_error(buf);
}

void __warning__(const char* func, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    char buf[2048];
    std::vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);
    migi::Platform::log(migi::Platform::LOG_LEVEL_WARNING, func, buf);
}

extern "C" uintptr_t make_generic_call_impl(uintptr_t procaddr, const uintptr_t* argsInRegisters, const void* inStack);
extern "C" void make_generic_call_r_mmx0(uintptr_t reg_cx, uintptr_t reg_dx, uintptr_t procaddr, const void* inStack, uint64_t* mmx0);

extern "C" uintptr_t ensureGILPreCall(uintptr_t)
{
    return static_cast<uintptr_t>(PyGILState_Ensure());
}

extern "C" void ensureGILPostCall(uintptr_t gilState)
{
    PyGILState_Release(static_cast<PyGILState_STATE>(gilState));
}

static uintptr_t make_generic_call(uintptr_t procaddr, const std::array<uintptr_t, 4>& argsInRegisters, const std::vector<uintptr_t>& argsInStacks, uint32_t callStackSize)
{
    std::vector<uintptr_t> sCallStacks(callStackSize ? callStackSize / sizeof(uintptr_t) : 2048);
    std::copy(argsInStacks.begin(), argsInStacks.end(), sCallStacks.end() - static_cast<intptr_t>(argsInStacks.size()));
    uintptr_t retcode = make_generic_call_impl(procaddr, argsInRegisters.data(), sCallStacks.data() + sCallStacks.size() - argsInStacks.size());
    return retcode;
}

static uint64_t makeGenericCallRMMX0(uintptr_t addr, const void* inStack, uint32_t inSize, uint32_t reg_cx, uint32_t reg_dx, uint32_t stackSize)
{
    uint8_t* stack = reinterpret_cast<uint8_t*>(malloc(stackSize));
    uint8_t* stackPointer = stack + stackSize - inSize;
    alignas (16) uint64_t mmx0;
    memcpy(stackPointer, inStack, inSize);
    make_generic_call_r_mmx0(reg_cx, reg_dx, addr, stackPointer, &mmx0);
    free(stack);
    return mmx0;
}

std::shared_ptr<Device> create_device(int32_t deviceType)
{
    return std::shared_ptr<Device>(Platform::createDevice(static_cast<Device::DeviceType>(deviceType)));
}

void sleep_for(float seconds)
{
    const py::GILScopedRelease release;
    std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<uint32_t>(seconds * 1000)));
}

bool is_mocked()
{
    return false;
}

uint32_t sizeof_void_p()
{
    return sizeof(uintptr_t);
}

std::string dump_bytes(const std::string& bytes) {
    return dumpMemory(reinterpret_cast<const uint8_t*>(bytes.c_str()), bytes.length());
}

uint64_t make_call(uintptr_t procAddr, const std::vector<uintptr_t>& args, uint32_t callStackSize)
{
    const py::GILScopedRelease release;
    const std::array<uintptr_t, 4> argsInRegisters = {0};
    return make_generic_call(procAddr, argsInRegisters, args, callStackSize);
}

uintptr_t stdcall_to_fastcall(uintptr_t stdcallFunctionPtr, bool ensureGIL, uintptr_t preCallPtr, uintptr_t postCallPtr)
{
#ifdef MIGI_ARCH_64
    FATAL("There is no fastcall in x64 arch");
#else
    StdcallToFastcallFunctionTable& functionTable = StdcallToFastcallFunctionTable::getInstance();
    if(ensureGIL)
        return functionTable.convert(stdcallFunctionPtr, ensureGILPreCall, ensureGILPostCall);
    return functionTable.convert(stdcallFunctionPtr, reinterpret_cast<FunctionTable::PreCallFunctionPtr>(preCallPtr), reinterpret_cast<FunctionTable::PostCallFunctionPtr>(postCallPtr));
#endif
}

void stdcall_to_fastcall_recycle(uintptr_t fastcallFunctionPtr)
{
#ifdef MIGI_ARCH_64
    FATAL("There is no fastcall in x64 arch");
#else
    StdcallToFastcallFunctionTable::getInstance().recycle(fastcallFunctionPtr);
#endif
}

uintptr_t stdcall_to_thiscall(uintptr_t stdcallFunctionPtr, bool ensureGIL, uintptr_t preCallPtr, uintptr_t postCallPtr)
{
    migi::StdcallToThiscallFunctionTable& functionTable = migi::StdcallToThiscallFunctionTable::getInstance();
    if(ensureGIL)
        return functionTable.convert(stdcallFunctionPtr, ensureGILPreCall, ensureGILPostCall);
    return functionTable.convert(stdcallFunctionPtr, reinterpret_cast<migi::FunctionTable::PreCallFunctionPtr>(preCallPtr), reinterpret_cast<migi::FunctionTable::PostCallFunctionPtr>(postCallPtr));
}

void stdcall_to_thiscall_recycle(uintptr_t fastcallFunctionPtr)
{
    StdcallToThiscallFunctionTable::getInstance().recycle(fastcallFunctionPtr);
}

uint64_t make_thiscall(uintptr_t procAddr, uintptr_t thisPtr, const std::vector<uintptr_t>& args, uint32_t callStackSize)
{
    const py::GILScopedRelease release;
    std::array<uintptr_t, 4> argsInRegisters = {thisPtr};
#ifdef MIGI_ARCH_64
    std::vector<uintptr_t> argsInStacks;
    std::copy(args.begin(), args.begin() + std::min<intptr_t>(3, static_cast<intptr_t>(args.size())), argsInRegisters.begin() + 1);
    if(args.size() > 3)
        std::copy(args.begin() + 3, args.end(), std::back_inserter(argsInStacks));
    return make_generic_call(procAddr, argsInRegisters, argsInStacks, callStackSize);
#else
    return make_generic_call(procAddr, argsInRegisters, args, callStackSize);
#endif
}

uint64_t make_fastcall(uintptr_t procAddr, uintptr_t arg0, uintptr_t arg1, const std::vector<uintptr_t>& args, uint32_t callStackSize)
{
#ifdef MIGI_ARCH_64
    FATAL("There is no fastcall in x64 arch");
#else
    const py::GILScopedRelease release;
    const std::array<uintptr_t, 4> argsInRegisters = {arg0, arg1};
    return make_generic_call(procAddr, argsInRegisters, args, callStackSize);
#endif
}

uint64_t make_call_r64(uintptr_t procAddr, uint32_t reg_cx, uint32_t reg_dx, uint32_t ss, const std::vector<uintptr_t>& stacks)
{
    const py::GILScopedRelease release;
    return makeGenericCallRMMX0(procAddr, stacks.data(), stacks.size() * sizeof(uintptr_t), reg_cx, reg_dx, ss ? ss : sizeof(uintptr_t) * 2048);
}

std::shared_ptr<Interceptor> make_interceptor(uintptr_t originalFunc, uintptr_t detoureFunc)
{
    return makeInterceptor(reinterpret_cast<void*>(originalFunc), reinterpret_cast<void*>(detoureFunc), nullptr);
}

uintptr_t load_library(const std::string& libraryName, uint32_t flags)
{
    return reinterpret_cast<uintptr_t>(Platform::loadLibrary(libraryName, flags));
}

void free_library(uintptr_t library)
{
    Platform::freeLibrary(reinterpret_cast<void*>(library));
}

void start_console(std::vector<std::string> commands)
{
    std::thread console(startConsole, std::move(commands));
    console.detach();
}

const std::string& get_module_file_path()
{
    static std::string sModuleFilePath;

    if(sModuleFilePath.empty())
        sModuleFilePath = Platform::getModulePath(reinterpret_cast<void*>(get_module_file_path));

    return sModuleFilePath;
}

void logd(const std::string& message)
{
    log(1, message);
}

uintptr_t get_module_address(const std::string& moduleName, uintptr_t offset)
{
    uintptr_t moduleAddr = reinterpret_cast<uintptr_t>(Platform::getModuleAddr(moduleName));
    return moduleAddr + offset;
}

uintptr_t get_module_proc(const std::string& moduleName, const std::string& procName)
{
    return reinterpret_cast<uintptr_t>(Platform::getModuleProcAddr(Platform::getModuleAddr(moduleName), procName));
}

const std::map<std::string, std::string>& get_properties()
{
    return gProperties;
}

}
