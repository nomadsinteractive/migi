#include <windows.h>

#include <iostream>
#include <fstream>
#include <filesystem>
#include <map>
#include <stdint.h>
#include <sstream>
#include <socketapi.h>
#include <string>
#include <thread>
#include <regex>
#include <inttypes.h>

#include <detours.h>
#include <nlohmann/json.hpp>

#include "migi.h"
#include "interceptor.h"
#include "module_function.h"
#include "console_std.h"

#include "intf/device.h"
#include "py/object.h"

#include "platform/platform.h"

namespace migi {


static uintptr_t gModule = 0;
static uintptr_t gExtraParameterPtr = 0;
static bool gConsoleMode = false;


bool gDetached = false;
std::map<std::string, std::string> gProperties;


std::string ensurePythonConsoleInput(Console& console)
{
    const py::GILScopedRelease release;
    while(!gDetached) {
        const std::string consoleInput = console.readLine();
        if(consoleInput.size() > 0)
            return consoleInput;
        std::this_thread::sleep_for(std::chrono::microseconds(1000));
    }
    return "";
}


std::string strip(const std::string& str)
{
    const char* front = str.c_str();
    const char* back = front + str.length() - 1;
    const char* wsFront = front;
    const char* wsBack = back;
    while(isspace(*wsFront))
        ++wsFront;
    while(wsBack >= wsFront && isspace(*wsBack))
        --wsBack;
    if(wsFront == front && wsBack == back)
        return str;
    return wsBack < wsFront ? std::string() : std::string(wsFront, wsBack + 1);
}

std::string dumpMemory(const uint8_t* memory, size_t length)
{
    char buf[256];
    char padding[] = "         ";
    std::ostringstream sb;

    for(size_t i = 0; i < length; i += 16)
    {
        std::snprintf(buf, sizeof(buf), "%08Xh: ", static_cast<uint32_t>(i));
        sb << buf;

        for(size_t j = 0; j < 16; j += 4)
        {
            size_t offset = i + j;
            const int32_t* p1 = reinterpret_cast<const int32_t*>(memory + offset);
            if(offset < length)
            {
                std::snprintf(buf, sizeof(buf), "%08X ", *p1);
                sb << buf;
            }
            else
                sb << padding;
        }

        sb << " |  ";
        for(size_t j = 0; j < 16; ++j)
        {
            size_t offset = i + j;
            if(offset < length)
                sb << (std::isprint(memory[offset]) ? static_cast<char>(memory[offset]) : '.');
            else
                sb << ' ';
        }
        sb << std::endl;
    }

    return sb.str();
}

static void cmdExit()
{
    DWORD dwThreadId;
    CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(FreeLibrary), reinterpret_cast<LPVOID>(gModule), 0, &dwThreadId);
}

void runConsoleInInteractiveMode(const std::vector<std::string>& runCommands)
{
    const std::unique_ptr<Device> device(Platform::createDevice(Device::DEVICE_TYPE_LOCAL_MACHINE));
    const std::unique_ptr<Console> console(device->createConsole());
    console->show();

    py::GILScopedAquire acquire;

    for(const std::string& i : runCommands)
        py::PyExec(i, *console);

    std::vector<std::string> scripts;
    while(!gDetached) {
        console->write(scripts.empty() ? ">>> " : "... ");
        std::string oneLine = ensurePythonConsoleInput(*console);
        std::string oneLineStripped = strip(oneLine);

        if(oneLineStripped == "exit")
        {
            cmdExit();
            break;
        }
        else if(oneLineStripped.length() > 0)
        {
            if(!scripts.empty())
                scripts.push_back(std::move(oneLine));
            else if(oneLineStripped.at(0) == '@' || oneLineStripped.at(oneLineStripped.length() - 1) == ':' || oneLineStripped.at(oneLineStripped.length() - 1) == '\\')
                scripts.push_back(std::move(oneLineStripped));
            else
                py::PyExec(oneLineStripped, *console);
        }
        else if(scripts.size() > 0)
        {
            std::ostringstream strbuf;
            for(const std::string& i : scripts)
                strbuf << i << '\n';
            py::PyExec(strbuf.str(), *console);
            scripts.clear();
        }
    }
}

void startConsole(std::vector<std::string> runCommands)
{
    gConsoleMode = true;
    runConsoleInInteractiveMode(runCommands);
    clearInterceptors();
}

std::filesystem::path tryAvailablePath(const std::filesystem::path& executableDir, const std::filesystem::path& workingDir, const std::filesystem::path& filepath)
{
    if(filepath.is_absolute())
        return filepath;

    const std::filesystem::path op1 = executableDir / filepath;
    if(std::filesystem::exists(op1))
        return op1;

    if(!workingDir.empty())
    {
        const std::filesystem::path op2 = workingDir / filepath;
        if(std::filesystem::exists(op2))
            return op2;
    }
    const std::filesystem::path currentDir = std::filesystem::absolute(std::filesystem::current_path());
    const std::filesystem::path op3(currentDir / filepath);
    return std::filesystem::exists(op3) ? op3 : std::filesystem::path();
}

static std::string regexpReplace(const std::string& s, const std::regex& pattern, const std::function<std::string(const std::smatch&)>& replacer)
{
    std::smatch match;
    std::string str = s;
    std::ostringstream strbuf;

    while(std::regex_search(str, match, pattern))
    {
        if(!match.prefix().str().empty())
            strbuf << match.prefix().str();
        strbuf << replacer(match);
        str = match.suffix().str();
    }
    if(!str.empty())
        strbuf << str;
    return strbuf.str();
}

static void initializeProperties(const std::filesystem::path& executableFilePath, const std::filesystem::path& manifestFilePath)
{
    gProperties["migi.executable_path"] = executableFilePath.string();
    gProperties["migi.executable_dir"] = executableFilePath.parent_path().string();

    if(!manifestFilePath.empty())
    {
        gProperties["migi.manifest_path"] = manifestFilePath.string();
        gProperties["migi.manifest_dir"] = manifestFilePath.parent_path().string();
    }
}

static std::string propertiesReplacer(const std::smatch& matcher)
{
    const std::string varName = matcher[1];
    if(varName == "migi.current_dir")
        return std::filesystem::absolute(std::filesystem::current_path()).string();

    const auto iter = gProperties.find(varName);
    return iter != gProperties.end() ? iter->second : "";
}

static std::string getVariable(const std::string& expression)
{
    static const std::regex VAR_PATTERN("\\$\\{([\\w.\\-]+)\\}");
    return regexpReplace(expression, VAR_PATTERN, propertiesReplacer);
}

static std::string getPathVariable(const std::filesystem::path& workDir, const std::string& expression)
{
    const std::filesystem::path path = getVariable(expression);
    return path.is_relative() ? (workDir / path).string() : path.string();
}

static std::vector<std::string> getJSONStringArray(const nlohmann::json& json, const std::string& name)
{
    const auto iter = json.find(name);
    if(iter == json.end() || !iter->is_array())
        return {};

    std::vector<std::string> retval;

    for(const nlohmann::json& i : *iter)
        if(i.is_string())
            retval.push_back(i.get<std::string>());

    return retval;
}

static std::map<std::string, std::string> getJSONStringDict(const nlohmann::json& json, const std::string& name)
{
    const auto iter = json.find(name);
    if(iter == json.end() || !iter->is_array())
        return {};

    std::map<std::string, std::string> retval;

    for(const nlohmann::json& i : *iter)
        if(i.is_object())
        {
            const auto jName = i.find("name");
            const auto jValue = i.find("value");
            if(jName != i.end() && jName->is_string() && jValue != i.end() && jValue->is_string())
                retval[jName->get<std::string>()] = jValue->get<std::string>();
        }

    return retval;
}

static void loadManifest(const nlohmann::json& manifest)
{
    const auto jWorkDir = manifest.find("work_dir");
    const std::filesystem::path workDir = jWorkDir != manifest.end() && jWorkDir->is_string() ? std::filesystem::path(getVariable(jWorkDir->get<std::string>())) : std::filesystem::current_path();
    gProperties["migi.work_dir"] = std::filesystem::absolute(workDir).string();

    for(const auto& [key, value]: getJSONStringDict(manifest, "properties"))
        gProperties[key] = getVariable(value);

    for(const std::string& i : getJSONStringArray(manifest, "load_libraries"))
        Platform::loadLibrary(std::filesystem::absolute(getPathVariable(workDir, i)).string(), 0);

    py::Object modsys = py::PyImportModule("sys");
    for(const std::string& i : getJSONStringArray(manifest, "python_paths"))
        modsys.attr("path").attr("append")(std::filesystem::absolute(getPathVariable(workDir, i)).string());

    const std::unique_ptr<Console> console(new ConsoleStd());
    for(const std::string& i : getJSONStringArray(manifest, "python_scripts"))
        py::PyRunScript(std::filesystem::absolute(getPathVariable(workDir, i)).string(), *console);

    if(manifest.find("python_console") != manifest.end())
    {
        std::thread consoleThread(startConsole, getJSONStringArray(manifest, "python_console"));
        consoleThread.detach();
    }
}

void start(int32_t argc, const char* argv[], uintptr_t module)
{
    gModule = module;

    if(argc > 0)
    {
        const std::filesystem::path executableFilePath = std::filesystem::absolute(std::filesystem::path(argv[0]));
        const std::filesystem::path executableDir = std::filesystem::absolute(executableFilePath.parent_path());

        nlohmann::json manifest;
        if(gExtraParameterPtr)
        {
            initializeProperties(executableFilePath, "");
            std::istringstream manifestInput(reinterpret_cast<const char*>(gExtraParameterPtr));
            manifestInput >> manifest;
        }
        else
        {
            const std::string execFileName = executableFilePath.stem().string();
            const auto dotPosition = execFileName.find('.');
            const std::string manifestFileName = dotPosition != std::string::npos ? execFileName.substr(0, dotPosition) : execFileName;
            const std::filesystem::path manifestFilePath = tryAvailablePath(executableDir, "", manifestFileName + ".json");
            initializeProperties(executableFilePath, manifestFilePath);

            std::ifstream manifestInput(manifestFilePath);
            if(manifestInput.good())
                manifestInput >> manifest;
        }

        std::unique_ptr<py::GILScopedAquire> aquire;
        const bool isInitialized = Py_IsInitialized();

        if(isInitialized)
            aquire = std::unique_ptr<py::GILScopedAquire>(new py::GILScopedAquire());
        else
            py::PyInitialize();

        py::List pyArgv;
        for(int32_t i = 0; i < argc; ++i)
            pyArgv.append(argv[i]);

        py::Object mod_sys = py::PyImportModule("sys");
        mod_sys.setattr("argv", pyArgv);

        loadManifest(manifest);

        {
            const py::GILScopedRelease release;
            while(!gDetached)
                std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<uint32_t>(2000)));
        }

        if(!isInitialized)
            py::PyFinalize();
    }
}

void log(uint32_t level, const std::string& message)
{
    const py::GILScopedRelease release;
    Platform::log(static_cast<Platform::LogLevel>(level), "[migi]", message);
}

bool isConsoleMode()
{
    return gConsoleMode;
}

void setExtraParameterPtr(uintptr_t extraParameterPtr)
{
    gExtraParameterPtr = extraParameterPtr;
}

}
