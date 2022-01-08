#ifndef MIGI_MIGI_H_
#define MIGI_MIGI_H_

#include <stdint.h>

#include <filesystem>

#include <string>
#include <vector>

#include <memory>
#include <string>

#include "api.h"

void start(int32_t argc, const char* argv[], uintptr_t module);
void detach();

void log(uint32_t level, const std::string& message);
bool isConsoleMode();

void startConsole(std::vector<std::string> runCommands);

std::string dumpMemory(const uint8_t* memory, size_t length);
void writeConsoleOutput(const std::string& output);

void setExtraParameterPtr(uintptr_t extraParameterPtr);

std::filesystem::path tryAvailablePath(const std::filesystem::path& executableDir, const std::filesystem::path& workingDir, const std::filesystem::path& filepath);

#endif
