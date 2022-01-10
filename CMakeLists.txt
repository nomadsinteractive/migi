cmake_minimum_required(VERSION 3.12)

project(migi LANGUAGES CXX C ASM_MASM)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(LOCAL_ARCH_64 TRUE)
    set(LOCAL_PROCESSOR_ARCH "win_amd64")
else()
    set(LOCAL_ARCH_64 FALSE)
    set(LOCAL_PROCESSOR_ARCH "win32")
endif()

find_package(Python3 REQUIRED COMPONENTS Interpreter Development)
string(REPLACE python${Python3_VERSION_MAJOR}${Python3_VERSION_MINOR}.lib python${Python3_VERSION_MAJOR}.lib Python3_LIMITED_API_LIBRARIES ${Python3_LIBRARIES})
message("Python3_FOUND:${Python3_FOUND}")
message("Python3_INCLUDE_DIRS:${Python3_INCLUDE_DIRS}")
message("Python3_VERSION:${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}.${Python3_VERSION_PATCH}")
message("Python3_Development_FOUND:${Python3_Development_FOUND}")
message("Python3_LIBRARIES:${Python3_LIBRARIES}")
message("Python3_LIMITED_API_LIBRARIES:${Python3_LIMITED_API_LIBRARIES}")

set(LOCAL_PYTHON_BINARY_SUFFIX .cp${Python3_VERSION_MAJOR}${Python3_VERSION_MINOR}-${LOCAL_PROCESSOR_ARCH})
set(LOCAL_PYTHON_LIMITED_API_BINARY_SUFFIX .cp${Python3_VERSION_MAJOR}-${LOCAL_PROCESSOR_ARCH})

add_library(detours STATIC IMPORTED)
if(LOCAL_ARCH_64)
    set_target_properties(detours PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/3rdparty/Detours/lib.X64/detours.lib)
else()
    set_target_properties(detours PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/3rdparty/Detours/lib.X86/detours.lib)
endif()

set(LOCAL_NATIVE_SRC_ROOT ${CMAKE_SOURCE_DIR}/src/native)
set(LOCAL_GENERATED_DIR_ROOT ${CMAKE_BINARY_DIR}/generated)

list(APPEND LOCAL_INCLUDE_DIRS ${LOCAL_NATIVE_SRC_ROOT})
list(APPEND LOCAL_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/3rdparty/Detours/include)
list(APPEND LOCAL_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/3rdparty/json/single_include)
list(APPEND LOCAL_INCLUDE_DIRS ${CMAKE_BINARY_DIR})
list(APPEND LOCAL_INCLUDE_DIRS ${Python3_INCLUDE_DIRS})

list(APPEND LOCAL_SRC_LIST
    ${LOCAL_NATIVE_SRC_ROOT}/api.h
    ${LOCAL_NATIVE_SRC_ROOT}/api.cc
    ${LOCAL_NATIVE_SRC_ROOT}/migi.h
    ${LOCAL_NATIVE_SRC_ROOT}/migi.cc
    ${LOCAL_NATIVE_SRC_ROOT}/module_function.h
    ${LOCAL_NATIVE_SRC_ROOT}/interceptor.h
    ${LOCAL_NATIVE_SRC_ROOT}/interceptor.cc
    ${LOCAL_NATIVE_SRC_ROOT}/console_std.h
    ${LOCAL_NATIVE_SRC_ROOT}/console_std.cc
    ${LOCAL_NATIVE_SRC_ROOT}/platform/platform.h
    ${LOCAL_NATIVE_SRC_ROOT}/forwarding.h
    ${LOCAL_NATIVE_SRC_ROOT}/function_table.h
    ${LOCAL_NATIVE_SRC_ROOT}/py/forwarding.h
    ${LOCAL_NATIVE_SRC_ROOT}/py/object.h
    ${LOCAL_NATIVE_SRC_ROOT}/py/object.cc
    ${LOCAL_NATIVE_SRC_ROOT}/py/python_api.h
    ${LOCAL_NATIVE_SRC_ROOT}/py/python_api.cc
    ${LOCAL_NATIVE_SRC_ROOT}/intf/console.h
    ${LOCAL_NATIVE_SRC_ROOT}/intf/device.h
    ${LOCAL_NATIVE_SRC_ROOT}/intf/injector.h

    ${LOCAL_NATIVE_SRC_ROOT}/platform/win32/platform_win32.cc
    ${LOCAL_NATIVE_SRC_ROOT}/platform/win32/console_win32.h
    ${LOCAL_NATIVE_SRC_ROOT}/platform/win32/console_win32.cc
    ${LOCAL_NATIVE_SRC_ROOT}/platform/win32/device_win32.h
    ${LOCAL_NATIVE_SRC_ROOT}/platform/win32/device_win32.cc
    ${LOCAL_NATIVE_SRC_ROOT}/platform/win32/injector_create_remote_thread.h
    ${LOCAL_NATIVE_SRC_ROOT}/platform/win32/injector_create_remote_thread.cc
)

macro(migi_add_debug_flag)
    string(FIND "${CMAKE_CXX_FLAGS_DEBUG}" "-DMIGI_FLAG_DEBUG" LOCAL_POS)
    if(LOCAL_POS EQUAL -1)
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DMIGI_FLAG_DEBUG")
    endif()
endmacro()

function(migi_add_resource_dir)
    set(LOCAL_RESOURCE_PATTERN "*.h" "*.hpp" "*.py" "*.xml" "*.json" "*.md" "*.yaml" "*.cmake" "*.template")
    foreach(i ${ARGN})
        foreach(j ${LOCAL_RESOURCE_PATTERN})
            list(APPEND LOCAL_GLOB_PATTERN ${i}/${j})
        endforeach()
    endforeach()
    file(GLOB_RECURSE LOCAL_GLOB_RESULT ${LOCAL_GLOB_PATTERN})
    list(APPEND LOCAL_RES_LIST ${LOCAL_GLOB_RESULT})
    set(LOCAL_RES_LIST ${LOCAL_RES_LIST} PARENT_SCOPE)
endfunction()

function(migi_add_target name)
    set(options SHARED_LIBRARY STATIC_LIBRARY EXECUTABLE)
    set(oneValueArgs OUTPUT_NAME PREFIX SUFFIX)
    set(multiValueArgs SRCS INCLUDE_DIRS COMPILE_DEFINITIONS COMPILER_OPTIONS LIBS LIB_DIRS)
    cmake_parse_arguments(MIGI_ADD_TARGET "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(MIGI_ADD_TARGET_SHARED_LIBRARY)
        add_library(${name} SHARED ${MIGI_ADD_TARGET_SRCS})
    elseif(MIGI_ADD_TARGET_STATIC_LIBRARY)
        add_library(${name} STATIC ${MIGI_ADD_TARGET_SRCS})
    elseif(MIGI_ADD_TARGET_EXECUTABLE)
        add_executable(${name} ${MIGI_ADD_TARGET_SRCS})
    else()
        message(FATAL_ERROR "Target ${name} has unspecified type")
    endif()

    if(MIGI_ADD_TARGET_INCLUDE_DIRS)
        target_include_directories(${name} PRIVATE ${MIGI_ADD_TARGET_INCLUDE_DIRS})
    endif()

    if(MIGI_ADD_TARGET_COMPILE_DEFINITIONS)
        target_compile_definitions(${name} PRIVATE ${MIGI_ADD_TARGET_COMPILE_DEFINITIONS})
    endif()

    if(MIGI_ADD_TARGET_COMPILER_OPTIONS)
        target_compile_options(${name} PRIVATE ${MIGI_ADD_TARGET_COMPILER_OPTIONS})
    endif()

    if(MIGI_ADD_TARGET_LIBS)
        target_link_libraries(${name} PRIVATE ${MIGI_ADD_TARGET_LIBS})
    endif()

    if(MIGI_ADD_TARGET_LIB_DIRS)
        target_link_directories(${name} PRIVATE ${MIGI_ADD_TARGET_LIB_DIRS})
    endif()

    if(MIGI_ADD_TARGET_OUTPUT_PREFIX)
        set_target_properties(${name} PROPERTIES PREFIX ${MIGI_ADD_TARGET_OUTPUT_PREFIX})
    endif()
    if(MIGI_ADD_TARGET_OUTPUT_NAME)
        set_target_properties(${name} PROPERTIES OUTPUT_NAME ${MIGI_ADD_TARGET_OUTPUT_NAME})
    endif()
    if(MIGI_ADD_TARGET_SUFFIX)
        set_target_properties(${name} PROPERTIES SUFFIX ${MIGI_ADD_TARGET_SUFFIX})
    endif()
endfunction()

function(migi_add_shared_library name)
    migi_add_target(${name} SHARED_LIBRARY ${ARGN})
endfunction()

function(migi_add_static_library name)
    migi_add_target(${name} STATIC_LIBRARY ${ARGN})
endfunction()

function(migi_add_executable name)
    migi_add_target(${name} EXECUTABLE ${ARGN})
endfunction()


macro(migi_add_generated_file GENERATOR OUTPUT_FILENAME WORKING_DIR DEPENDENCY_LIST)
if(${GENERATOR} MATCHES "[.]py$")
    set(LOCAL_GENERATOR_COMMAND python ${GENERATOR})
else()
    set(LOCAL_GENERATOR_COMMAND ${GENERATOR})
endif()

set(LOCAL_OUTPUT_FILENAME_ABSOLUTE ${LOCAL_GENERATED_DIR_ROOT}/${OUTPUT_FILENAME})
add_custom_command(OUTPUT ${LOCAL_OUTPUT_FILENAME_ABSOLUTE}
    COMMAND ${LOCAL_GENERATOR_COMMAND} ${ARGN} ${LOCAL_OUTPUT_FILENAME_ABSOLUTE}
    DEPENDS ${DEPENDENCY_LIST} ${GENERATOR}
    WORKING_DIRECTORY ${WORKING_DIR})
list(APPEND LOCAL_GENERATED_SRC_LIST ${LOCAL_OUTPUT_FILENAME_ABSOLUTE})
endmacro()

migi_add_resource_dir(${LOCAL_NATIVE_SRC_ROOT} examples tools)

list(APPEND LOCAL_LIBS detours ws2_32 iphlpapi psapi)

set(MIGI_FUNC_TABLE_LEN 64 CACHE INTEGER "Length of FunctionTable")

macro(migi_gen_func_table TABLE_NAME INSTRUCTION_TYPE REG_ARGS)
    migi_add_generated_file(${CMAKE_SOURCE_DIR}/tools/gen_func_table.py ${TABLE_NAME}_${INSTRUCTION_TYPE}.asm ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/tools/func_table.${INSTRUCTION_TYPE}.template -n ${TABLE_NAME}  -l ${MIGI_FUNC_TABLE_LEN} -t tools/func_table.${INSTRUCTION_TYPE}.template -r ${REG_ARGS})
    migi_add_generated_file(${CMAKE_SOURCE_DIR}/tools/gen_func_table.py ${TABLE_NAME}_function_table.h ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/tools/func_table.h.template -n ${TABLE_NAME}  -l ${MIGI_FUNC_TABLE_LEN} -t tools/func_table.h.template)
    migi_add_generated_file(${CMAKE_SOURCE_DIR}/tools/gen_func_table.py ${TABLE_NAME}_function_table.cc ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/tools/func_table.cc.template -n ${TABLE_NAME}  -l ${MIGI_FUNC_TABLE_LEN} -t tools/func_table.cc.template)
endmacro()

if(LOCAL_ARCH_64)
    list(APPEND LOCAL_SRC_LIST
        ${LOCAL_NATIVE_SRC_ROOT}/m64.asm
    )
    migi_gen_func_table(stdcall_to_thiscall m64 rcx)
else()
    list(APPEND LOCAL_SRC_LIST
        ${LOCAL_NATIVE_SRC_ROOT}/m32.asm
    )
    migi_gen_func_table(stdcall_to_fastcall m32 edx,ecx)
    migi_gen_func_table(stdcall_to_thiscall m32 ecx)
endif()

migi_add_debug_flag()

list(APPEND LOCAL_COMPILE_DEFINITIONS
    Py_LIMITED_API
)


migi_add_static_library(
    migi_static
    SRCS ${LOCAL_SRC_LIST} ${LOCAL_GENERATED_SRC_LIST} ${LOCAL_RES_LIST}
    SRCS ${LOCAL_NATIVE_SRC_ROOT}/py/_migimodule.cc
    INCLUDE_DIRS ${LOCAL_INCLUDE_DIRS}
    COMPILE_DEFINITIONS ${LOCAL_COMPILE_DEFINITIONS}
    COMPILE_OPTIONS ${LOCAL_COMPILER_OPTIONS}
)

migi_add_shared_library(
    migi
    SRCS ${LOCAL_NATIVE_SRC_ROOT}/platform/win32/dllmain.cc
    LIBS ${LOCAL_LIBS} ${Python3_LIMITED_API_LIBRARIES} migi_static
    LIB_DIRS ${Python3_LIBRARY_DIRS}
    INCLUDE_DIRS ${LOCAL_INCLUDE_DIRS}
    COMPILE_DEFINITIONS ${LOCAL_COMPILE_DEFINITIONS}
    COMPILE_OPTIONS ${LOCAL_COMPILER_OPTIONS}
    OUTPUT_NAME migi${LOCAL_PYTHON_LIMITED_API_BINARY_SUFFIX}
)

migi_add_shared_library(
    migi_pyd_limited_api
    SRCS ${LOCAL_NATIVE_SRC_ROOT}/py/_migimodule.cc
    LIBS ${LOCAL_LIBS} ${Python3_LIBRARIES} migi_static
    LIB_DIRS ${Python3_LIBRARY_DIRS}
    INCLUDE_DIRS ${LOCAL_INCLUDE_DIRS}
    COMPILE_DEFINITIONS ${LOCAL_COMPILE_DEFINITIONS}
    COMPILE_OPTIONS ${LOCAL_COMPILER_OPTIONS}
    OUTPUT_NAME _migi${LOCAL_PYTHON_LIMITED_API_BINARY_SUFFIX}
    SUFFIX .pyd
)

migi_add_executable(
    migi_launcher
    SRCS ${LOCAL_NATIVE_SRC_ROOT}/main.cc
    LIBS ${LOCAL_LIBS} ${Python3_LIBRARIES} migi_static
    LIB_DIRS ${Python3_LIBRARY_DIRS}
    OUTPUT_NAME migi
    INCLUDE_DIRS ${LOCAL_INCLUDE_DIRS}
    COMPILE_DEFINITIONS ${LOCAL_COMPILE_DEFINITIONS} MIGI_FLAG_EMBED_MODULE
    COMPILE_OPTIONS ${LOCAL_COMPILER_OPTIONS}
)

install(TARGETS migi RUNTIME DESTINATION bin COMPONENT migi_injector)
install(TARGETS migi_pyd_limited_api RUNTIME DESTINATION bin COMPONENT migi_bindings_limited_api)
