cmake_minimum_required(VERSION 3.25)
project(Alpine VERSION 0.0.19 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Output directories
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Options
option(ALPINE_ENABLE_CORBA "Enable CORBA remote management (requires ACE/TAO)" OFF)
option(ALPINE_BUILD_TESTS "Build test programs" ON)
set(ALPINE_SANITIZER "" CACHE STRING
    "Enable sanitizer instrumentation (address, undefined, thread, memory, or comma-separated combo)")

# Custom Profile build type
set(CMAKE_CXX_FLAGS_PROFILE "-pg -g -D_PROFILE" CACHE STRING "Flags for Profile build type")
set(CMAKE_EXE_LINKER_FLAGS_PROFILE "-pg" CACHE STRING "Linker flags for Profile build type")
set(CMAKE_SHARED_LINKER_FLAGS_PROFILE "-pg" CACHE STRING "Shared linker flags for Profile build type")

# Common flags interface target
add_library(AlpineFlags INTERFACE)
add_library(Alpine::Flags ALIAS AlpineFlags)
if(WIN32)
    target_compile_options(AlpineFlags INTERFACE -Wall)
    target_compile_definitions(AlpineFlags INTERFACE
        WIN32_LEAN_AND_MEAN
        _WIN32_WINNT=0x0600
        NOMINMAX
        _CONCURRENT
        _VERBOSE
    )
else()
    target_compile_options(AlpineFlags INTERFACE -Wall -fPIC)
    target_compile_definitions(AlpineFlags INTERFACE
        _GNU_SOURCE
        _POSIX_THREADS
        _CONCURRENT
        _REENTRANT
        _VERBOSE
    )
endif()

# Sanitizer support (requires Clang or GCC >= 4.8)
if(ALPINE_SANITIZER)
    target_compile_options(AlpineFlags INTERFACE
        -fsanitize=${ALPINE_SANITIZER}
        -fno-omit-frame-pointer
        -fno-optimize-sibling-calls
    )
    target_link_options(AlpineFlags INTERFACE
        -fsanitize=${ALPINE_SANITIZER}
    )
    message(STATUS "Sanitizer enabled: ${ALPINE_SANITIZER}")
endif()

find_package(Threads REQUIRED)
target_link_libraries(AlpineFlags INTERFACE Threads::Threads)

if(WIN32)
    target_link_libraries(AlpineFlags INTERFACE ws2_32 iphlpapi)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(AlpineFlags INTERFACE rt dl)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_link_libraries(AlpineFlags INTERFACE dl)
endif()

# Base include path (for Common.h)
target_include_directories(AlpineFlags INTERFACE ${CMAKE_SOURCE_DIR}/base)

# Base libraries
add_subdirectory(base/AppUtils)
add_subdirectory(base/SysUtils)
add_subdirectory(base/ThreadUtils)
add_subdirectory(base/NetUtils)
add_subdirectory(base/ConfigUtils)

# Transport layer
add_subdirectory(transport/TransBase)
add_subdirectory(transport/Dtcp)
add_subdirectory(transport/Alpine)

# Protocol layer
add_subdirectory(protocols/Alpine)

# Application core
add_subdirectory(applcore/ApplCore)

# Interface facades
add_subdirectory(interfaces/AlpineStackInterface)
add_subdirectory(interfaces/DtcpStackInterface)

# CORBA (optional)
if(ALPINE_ENABLE_CORBA)
    list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
    find_package(TAO REQUIRED)
    add_subdirectory(corba/idl/AlpineIdl)
    add_subdirectory(corba/CorbaUtils)
    add_subdirectory(corba/CorbaAdmin)
    add_subdirectory(corba/CorbaServant)
    add_subdirectory(corba/interfaces/AlpineCorbaServer)
    add_subdirectory(corba/interfaces/AlpineCorbaClient)
endif()

# Binaries
add_subdirectory(AlpineServer)
add_subdirectory(AlpineCmdIf)
add_subdirectory(AlpineRestBridge)

# Tests
if(ALPINE_BUILD_TESTS)
    add_subdirectory(test/auto_thread_test)
    add_subdirectory(test/sys_thread_test)
    add_subdirectory(test/map_test)
    add_subdirectory(test/spawn_process)
    add_subdirectory(test/tcp_conn_test)
    add_subdirectory(test/udp_send_test)
    add_subdirectory(test/udp_socket_conn)
    add_subdirectory(test/dtcp_stack)
    add_subdirectory(test/dl_test/DynLib)
    add_subdirectory(test/dl_test/DynamicLoaderTest)
    if(ALPINE_ENABLE_CORBA)
        add_subdirectory(test/corba_util_test)
    endif()
endif()
