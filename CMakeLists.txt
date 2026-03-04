cmake_minimum_required(VERSION 3.28)
project(Alpine VERSION 0.0.19 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Output directories
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Options
option(ALPINE_ENABLE_CORBA "Enable CORBA remote management (requires ACE/TAO)" OFF)
option(ALPINE_ENABLE_TLS "Enable TLS/DTLS encryption" OFF)
option(ALPINE_BUILD_TESTS "Build test programs" ON)
option(ALPINE_BUILD_FUZZERS "Build libFuzzer targets (requires Clang)" OFF)
option(ALPINE_BUILD_BENCHMARKS "Build benchmark programs" OFF)
option(ALPINE_ENABLE_PERSISTENCE "Enable SQLite persistence layer" OFF)
option(ALPINE_ENABLE_TRACING "Enable OpenTelemetry distributed tracing" OFF)
option(ALPINE_ENABLE_UPNP "Enable UPnP IGD port mapping" OFF)
option(ALPINE_ENABLE_FUSE "Enable FUSE virtual filesystem" OFF)
option(ALPINE_USE_SYSTEM_DEPS "Use system-installed dependencies via find_package instead of FetchContent" OFF)
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
    target_compile_options(AlpineFlags INTERFACE -Wall)
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

if(ALPINE_ENABLE_TLS)
    find_package(OpenSSL REQUIRED)
endif()

include(FetchContent)

if(ALPINE_USE_SYSTEM_DEPS)
    # Use system-installed packages (vcpkg, Conan, or manual install)
    find_package(nlohmann_json REQUIRED)
    find_package(spdlog REQUIRED)
    find_package(asio CONFIG QUIET)
    if(NOT asio_FOUND)
        # Fallback: asio may be header-only in a system path
        find_path(ASIO_INCLUDE_DIR asio.hpp REQUIRED)
    endif()

    if(ALPINE_ENABLE_TLS)
        find_package(jwt-cpp REQUIRED)
    endif()

    add_library(asio_standalone INTERFACE)
    add_library(Asio::Asio ALIAS asio_standalone)
    if(asio_FOUND)
        target_link_libraries(asio_standalone INTERFACE asio::asio)
    else()
        target_include_directories(asio_standalone INTERFACE ${ASIO_INCLUDE_DIR})
    endif()
    target_compile_definitions(asio_standalone INTERFACE ASIO_STANDALONE ASIO_NO_DEPRECATED)
else()
    FetchContent_Declare(json
        URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
        URL_HASH SHA256=d6c65aca6b1ed68e7a182f4757257b107ae403032760ed6ef121c9d55e81757d
    )
    FetchContent_MakeAvailable(json)

    FetchContent_Declare(spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.14.1
        GIT_SHALLOW TRUE
    )
    set(SPDLOG_FMT_EXTERNAL OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(spdlog)

    FetchContent_Declare(asio
        GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
        GIT_TAG asio-1-30-2
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(asio)

    if(ALPINE_ENABLE_TLS)
        FetchContent_Declare(jwt-cpp
            GIT_REPOSITORY https://github.com/Thalhammer/jwt-cpp.git
            GIT_TAG v0.7.0
            GIT_SHALLOW TRUE
        )
        FetchContent_MakeAvailable(jwt-cpp)
    endif()

    add_library(asio_standalone INTERFACE)
    add_library(Asio::Asio ALIAS asio_standalone)
    target_include_directories(asio_standalone INTERFACE ${asio_SOURCE_DIR}/asio/include)
    target_compile_definitions(asio_standalone INTERFACE ASIO_STANDALONE ASIO_NO_DEPRECATED)
endif()

if(WIN32)
    target_link_libraries(AlpineFlags INTERFACE ws2_32 iphlpapi)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(AlpineFlags INTERFACE rt dl)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_link_libraries(AlpineFlags INTERFACE dl)
endif()

# UPnP IGD port mapping (optional)
if(ALPINE_ENABLE_UPNP)
    FetchContent_Declare(miniupnpc
        GIT_REPOSITORY https://github.com/miniupnp/miniupnp.git
        GIT_TAG miniupnpc_2_2_7
        GIT_SHALLOW TRUE
        SOURCE_SUBDIR miniupnpc
    )
    set(UPNPC_BUILD_SHARED OFF CACHE BOOL "" FORCE)
    set(UPNPC_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(UPNPC_BUILD_SAMPLE OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(miniupnpc)
endif()

# SQLite persistence (optional)
if(ALPINE_ENABLE_PERSISTENCE)
    FetchContent_Declare(sqlite3
        URL https://www.sqlite.org/2024/sqlite-amalgamation-3450000.zip
        URL_HASH SHA256=e6e0e3f9ccbfcf34bc30d5e565ea4e0dab4e209e0be0f5f3c2e28658e8e75e38
    )
    FetchContent_MakeAvailable(sqlite3)

    add_library(sqlite3_lib ${sqlite3_SOURCE_DIR}/sqlite3.c)
    target_include_directories(sqlite3_lib PUBLIC ${sqlite3_SOURCE_DIR})
    target_compile_definitions(sqlite3_lib PRIVATE SQLITE_THREADSAFE=1)
    set_target_properties(sqlite3_lib PROPERTIES C_STANDARD 11)

    add_library(PersistenceStore
        base/PersistenceStore.cpp
    )
    target_include_directories(PersistenceStore PUBLIC ${CMAKE_SOURCE_DIR}/base)
    target_link_libraries(PersistenceStore PUBLIC Alpine::AppUtils sqlite3_lib)
    add_library(Alpine::PersistenceStore ALIAS PersistenceStore)
endif()

# FUSE virtual filesystem (optional)
if(ALPINE_ENABLE_FUSE)
    list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
    find_package(FUSE REQUIRED)
endif()

# OpenTelemetry tracing (optional)
if(ALPINE_ENABLE_TRACING)
    FetchContent_Declare(opentelemetry-cpp
        GIT_REPOSITORY https://github.com/open-telemetry/opentelemetry-cpp.git
        GIT_TAG v1.14.2
        GIT_SHALLOW TRUE
    )
    set(WITH_OTLP_HTTP ON CACHE BOOL "" FORCE)
    set(WITH_OTLP_GRPC OFF CACHE BOOL "" FORCE)
    set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(opentelemetry-cpp)
endif()

# Base include path (for Common.h)
target_include_directories(AlpineFlags INTERFACE ${CMAKE_SOURCE_DIR}/base)

# Base libraries
add_subdirectory(base/AppUtils)
add_subdirectory(base/SysUtils)
add_subdirectory(base/ThreadUtils)
add_subdirectory(base/NetUtils)
add_subdirectory(base/ConfigUtils)
add_subdirectory(base/Tracing)
if(ALPINE_ENABLE_FUSE)
    add_subdirectory(base/VfsUtils)
endif()

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
add_subdirectory(AlpineGui)

# Tests
if(ALPINE_BUILD_TESTS)
    enable_testing()

    if(ALPINE_USE_SYSTEM_DEPS)
        find_package(Catch2 3 REQUIRED)
    else()
        FetchContent_Declare(Catch2
            GIT_REPOSITORY https://github.com/catchorg/Catch2.git
            GIT_TAG v3.5.2
            GIT_SHALLOW TRUE
        )
        FetchContent_MakeAvailable(Catch2)
        list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
    endif()
    include(CTest)
    include(Catch)

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
    add_subdirectory(test/wifi_hwsim)
    if(ALPINE_ENABLE_CORBA)
        add_subdirectory(test/corba_util_test)
    endif()

    add_subdirectory(test/unit)
endif()

# Fuzzing targets (requires Clang with libFuzzer)
if(ALPINE_BUILD_FUZZERS)
    add_subdirectory(fuzz)
endif()

# Benchmarks
if(ALPINE_BUILD_BENCHMARKS)
    if(ALPINE_USE_SYSTEM_DEPS)
        find_package(benchmark REQUIRED)
    else()
        FetchContent_Declare(benchmark
            GIT_REPOSITORY https://github.com/google/benchmark.git
            GIT_TAG v1.8.3
            GIT_SHALLOW TRUE
        )
        set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(benchmark)
    endif()
    add_subdirectory(bench)
endif()

# ---------------------------------------------------------------------------
# Install rules
# ---------------------------------------------------------------------------
include(GNUInstallDirs)

# Executables
install(TARGETS AlpineServer AlpineCmdIf AlpineRestBridge
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Libraries
install(TARGETS
    AppUtils SysUtils ThreadUtils NetUtils ConfigUtils
    TransBase Dtcp AlpineTransport
    AlpineProtocol
    ApplCore
    AlpineStackInterface DtcpStackInterface
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Public headers
install(FILES base/Common.h base/Platform.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/alpine
)
install(DIRECTORY base/AppUtils/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/alpine/AppUtils FILES_MATCHING PATTERN "*.h")
install(DIRECTORY base/SysUtils/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/alpine/SysUtils FILES_MATCHING PATTERN "*.h")
install(DIRECTORY base/ThreadUtils/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/alpine/ThreadUtils FILES_MATCHING PATTERN "*.h")
install(DIRECTORY base/NetUtils/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/alpine/NetUtils FILES_MATCHING PATTERN "*.h")
install(DIRECTORY base/ConfigUtils/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/alpine/ConfigUtils FILES_MATCHING PATTERN "*.h")
install(DIRECTORY transport/TransBase/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/alpine/TransBase FILES_MATCHING PATTERN "*.h")
install(DIRECTORY transport/Dtcp/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/alpine/Dtcp FILES_MATCHING PATTERN "*.h")
install(DIRECTORY transport/Alpine/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/alpine/AlpineTransport FILES_MATCHING PATTERN "*.h")
install(DIRECTORY protocols/Alpine/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/alpine/AlpineProtocol FILES_MATCHING PATTERN "*.h")
install(DIRECTORY applcore/ApplCore/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/alpine/ApplCore FILES_MATCHING PATTERN "*.h")
install(DIRECTORY interfaces/AlpineStackInterface/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/alpine/AlpineStackInterface FILES_MATCHING PATTERN "*.h")
install(DIRECTORY interfaces/DtcpStackInterface/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/alpine/DtcpStackInterface FILES_MATCHING PATTERN "*.h")

# ---------------------------------------------------------------------------
# CPack configuration
# ---------------------------------------------------------------------------
set(CPACK_PACKAGE_NAME "alpine")
set(CPACK_PACKAGE_VENDOR "sonoransun")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Alpine — Decentralized P2P resource discovery platform")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_SOURCE_DIR}/LICENCE.txt)
set(CPACK_PACKAGE_CONTACT "sonoransun")

# Generator defaults
set(CPACK_GENERATOR "TGZ")

# DEB settings
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "sonoransun")
set(CPACK_DEBIAN_PACKAGE_SECTION "net")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

# RPM settings
set(CPACK_RPM_PACKAGE_LICENSE "LGPL")
set(CPACK_RPM_PACKAGE_GROUP "Applications/Internet")

include(CPack)
