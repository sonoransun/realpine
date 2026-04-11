cmake_minimum_required(VERSION 3.28)
project(Alpine VERSION 0.0.19 LANGUAGES CXX C)

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
option(ALPINE_ENABLE_COVERAGE "Enable code coverage instrumentation" OFF)
option(ALPINE_USE_SYSTEM_DEPS "Use system-installed dependencies via find_package instead of FetchContent" OFF)
option(ALPINE_WARNINGS_AS_ERRORS "Promote compiler warnings to errors (CI uses this)" OFF)
option(ALPINE_ENABLE_LTO "Enable link-time optimization for Release builds" OFF)
option(ALPINE_INSTALL_SYSTEMD "Install systemd unit file (Linux only)" OFF)
set(ALPINE_SANITIZER "" CACHE STRING
    "Enable sanitizer instrumentation (address, undefined, thread, memory, or comma-separated combo)")

# Custom Profile build type
set(CMAKE_CXX_FLAGS_PROFILE "-pg -g -D_PROFILE" CACHE STRING "Flags for Profile build type")
set(CMAKE_EXE_LINKER_FLAGS_PROFILE "-pg" CACHE STRING "Linker flags for Profile build type")
set(CMAKE_SHARED_LINKER_FLAGS_PROFILE "-pg" CACHE STRING "Shared linker flags for Profile build type")

# Common flags interface target
add_library(AlpineFlags INTERFACE)
add_library(Alpine::Flags ALIAS AlpineFlags)

# Warning set — applies to every Alpine translation unit.  The curated set
# below catches real bugs in this codebase without requiring the long-tail
# shadow/conversion cleanup that -Wshadow / -Wconversion would demand.
# `-Werror` is opt-in via ALPINE_WARNINGS_AS_ERRORS so local developer
# builds remain warning-friendly while CI stays strict.
set(ALPINE_WARNING_FLAGS
    -Wall
    -Wextra
    -Wpedantic
    -Wformat=2
    -Wnon-virtual-dtor
    -Wimplicit-fallthrough
    -Wmisleading-indentation
    -Wdouble-promotion
    -Wno-unused-parameter
    -Wno-unused-variable
    -Wno-unused-function
    -Wno-unused-result
    -Wno-sign-compare
    -Wno-missing-field-initializers
    -Wno-deprecated-declarations
    -Wno-deprecated-copy
    -Wno-missing-braces
    -Wno-reorder
)
# These warning-disables exist only on GCC.  Clang rejects `-Wno-class-memaccess`
# and `-Wno-maybe-uninitialized` as "unknown warning option" under `-Werror`,
# so gate them on compiler id.
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    list(APPEND ALPINE_WARNING_FLAGS
        -Wno-class-memaccess
        -Wno-maybe-uninitialized
        -Wno-unused-but-set-variable
    )
endif()
# Clang's `-Werror` turns unrecognized `-Wno-*` flags into errors; accept them
# instead so we can keep a single shared flag list across compilers.
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    list(APPEND ALPINE_WARNING_FLAGS -Wno-unknown-warning-option)
endif()

if(ALPINE_WARNINGS_AS_ERRORS)
    list(APPEND ALPINE_WARNING_FLAGS -Werror)
    message(STATUS "ALPINE_WARNINGS_AS_ERRORS: warnings are fatal")
endif()

if(WIN32)
    target_compile_options(AlpineFlags INTERFACE ${ALPINE_WARNING_FLAGS})
    target_compile_definitions(AlpineFlags INTERFACE
        WIN32_LEAN_AND_MEAN
        _WIN32_WINNT=0x0600
        NOMINMAX
        _CONCURRENT
        _VERBOSE
    )
else()
    target_compile_options(AlpineFlags INTERFACE ${ALPINE_WARNING_FLAGS})
    target_compile_definitions(AlpineFlags INTERFACE
        _GNU_SOURCE
        _POSIX_THREADS
        _CONCURRENT
        _REENTRANT
        _VERBOSE
    )
endif()

# Linux Release-mode hardening flags.  `-D_FORTIFY_SOURCE=2` requires at least
# `-O1`, so these are gated on Release / RelWithDebInfo / Profile.  The
# generator expression keeps Debug builds debuggable.
#
# `-fPIE`/`-pie` MUST only be applied to executables.  If we push them into
# an INTERFACE library that is consumed by SHARED targets, the resulting
# objects end up with PIE-only TLS relocations (R_X86_64_TPOFF32) that the
# linker rejects with "can not be used when making a shared object".  Gate
# them on TARGET_PROPERTY:TYPE so that CMake's own PIC handling
# (CMAKE_POSITION_INDEPENDENT_CODE=ON) supplies `-fPIC` for libraries and we
# only add `-fPIE`/`-pie` on top for EXECUTABLE consumers.
if(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT WIN32)
    target_compile_options(AlpineFlags INTERFACE
        $<$<NOT:$<CONFIG:Debug>>:-fstack-protector-strong>
        $<$<NOT:$<CONFIG:Debug>>:-D_FORTIFY_SOURCE=2>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:-fPIE>
    )
    target_link_options(AlpineFlags INTERFACE
        -Wl,-z,relro
        -Wl,-z,now
        -Wl,-z,noexecstack
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:-pie>
    )
endif()

# Link-time optimization (opt-in).  Disable for Debug to keep build times sane.
if(ALPINE_ENABLE_LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT ALPINE_IPO_SUPPORTED OUTPUT ALPINE_IPO_ERROR)
    if(ALPINE_IPO_SUPPORTED)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO TRUE)
        message(STATUS "LTO / IPO enabled for Release builds")
    else()
        message(WARNING "LTO requested but not supported by compiler: ${ALPINE_IPO_ERROR}")
    endif()
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

if(ALPINE_ENABLE_COVERAGE)
    # `-fprofile-update=atomic` avoids the "negative count" gcov error that
    # libstdc++'s heavily-inlined <format> / <string> produce when multiple
    # threads update the same counter concurrently (as they do in Catch2's
    # test harness).
    target_compile_options(AlpineFlags INTERFACE --coverage -O0 -g -fprofile-update=atomic)
    target_link_options(AlpineFlags INTERFACE --coverage)
    message(STATUS "Code coverage instrumentation enabled")

    # `make coverage` → run ctest, capture .gcda, filter out third-party,
    # emit an HTML report under build/coverage/.  Requires lcov + genhtml.
    find_program(LCOV_PROGRAM   NAMES lcov)
    find_program(GENHTML_PROGRAM NAMES genhtml)
    if(LCOV_PROGRAM AND GENHTML_PROGRAM)
        # gcov version has to match the compiler that produced the .gcno files.
        # lcov defaults to whatever `gcov` is on PATH, which on this host is
        # gcc-13's gcov — not the gcc-14 we actually compile with.  Pick a
        # matching gcov tool from the same toolchain as CMAKE_CXX_COMPILER.
        get_filename_component(_cxx_dir  "${CMAKE_CXX_COMPILER}" DIRECTORY)
        get_filename_component(_cxx_name "${CMAKE_CXX_COMPILER}" NAME)
        string(REGEX REPLACE "^g\\+\\+" "gcov" _gcov_name "${_cxx_name}")
        find_program(ALPINE_GCOV_TOOL NAMES ${_gcov_name} gcov-14 gcov HINTS ${_cxx_dir})
        set(_gcov_arg "")
        if(ALPINE_GCOV_TOOL)
            set(_gcov_arg "--gcov-tool" "${ALPINE_GCOV_TOOL}")
            message(STATUS "Coverage gcov tool: ${ALPINE_GCOV_TOOL}")
        endif()
        add_custom_target(coverage
            COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --timeout 180 || true
            COMMAND ${LCOV_PROGRAM} --capture --directory ${CMAKE_BINARY_DIR}
                --output-file ${CMAKE_BINARY_DIR}/coverage.info --quiet
                ${_gcov_arg}
                --rc geninfo_unexecuted_blocks=1
                --ignore-errors mismatch,unused,empty,source,version,inconsistent,negative,gcov,format
            COMMAND ${LCOV_PROGRAM} --remove ${CMAKE_BINARY_DIR}/coverage.info
                "'*/_deps/*'" "'*/build*/*'" "'*/test/*'" "'/usr/*'"
                --output-file ${CMAKE_BINARY_DIR}/coverage.info --quiet
                ${_gcov_arg}
                --ignore-errors unused,inconsistent
            COMMAND ${GENHTML_PROGRAM} ${CMAKE_BINARY_DIR}/coverage.info
                --output-directory ${CMAKE_BINARY_DIR}/coverage --quiet
                --ignore-errors source,unmapped,inconsistent,category,negative
            COMMENT "Generating coverage report in ${CMAKE_BINARY_DIR}/coverage/"
            VERBATIM
            USES_TERMINAL
        )
    else()
        message(WARNING "Coverage enabled but lcov/genhtml not found — 'coverage' target not created")
    endif()
endif()

find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)
target_link_libraries(AlpineFlags INTERFACE Threads::Threads)

# Probe for optional platform symbols so we can take the fast paths when
# the host libc exposes them and fall back cleanly elsewhere.
# NB: several of these (explicit_bzero, accept4, pthread_setname_np) are
# glibc extensions guarded by _GNU_SOURCE, so the probe must see that define
# even though AlpineFlags adds it to real build targets later.
include(CheckSymbolExists)
set(_ALPINE_PROBE_DEFS_SAVED "${CMAKE_REQUIRED_DEFINITIONS}")
if(NOT WIN32)
    list(APPEND CMAKE_REQUIRED_DEFINITIONS -D_GNU_SOURCE)
endif()
# On glibc, explicit_bzero is declared in <string.h> (guarded by __USE_MISC),
# not <strings.h>. On BSD libc it lives in <strings.h>. Probe both headers so
# the check fires on either host.
check_symbol_exists(explicit_bzero "string.h;strings.h" HAVE_EXPLICIT_BZERO)
check_symbol_exists(getrandom "sys/random.h" HAVE_GETRANDOM)
check_symbol_exists(accept4 "sys/socket.h" HAVE_ACCEPT4)
check_symbol_exists(pthread_setname_np "pthread.h" HAVE_PTHREAD_SETNAME_NP)
set(CMAKE_REQUIRED_DEFINITIONS "${_ALPINE_PROBE_DEFS_SAVED}")
unset(_ALPINE_PROBE_DEFS_SAVED)

target_compile_definitions(AlpineFlags INTERFACE
    $<$<BOOL:${HAVE_EXPLICIT_BZERO}>:ALPINE_HAVE_EXPLICIT_BZERO=1>
    $<$<BOOL:${HAVE_GETRANDOM}>:ALPINE_HAVE_GETRANDOM=1>
    $<$<BOOL:${HAVE_ACCEPT4}>:ALPINE_HAVE_ACCEPT4=1>
    $<$<BOOL:${HAVE_PTHREAD_SETNAME_NP}>:ALPINE_HAVE_PTHREAD_SETNAME_NP=1>
)

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

# linenoise for interactive CLI (BSD-licensed, single-file C library)
FetchContent_Declare(linenoise
    GIT_REPOSITORY https://github.com/antirez/linenoise.git
    GIT_TAG master
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(linenoise)

# antirez/linenoise has no CMakeLists.txt, so we create a library target.
if(NOT TARGET linenoise)
    add_library(linenoise STATIC ${linenoise_SOURCE_DIR}/linenoise.c)
    target_include_directories(linenoise PUBLIC ${linenoise_SOURCE_DIR})
    set_target_properties(linenoise PROPERTIES C_STANDARD 11)
endif()

if(WIN32)
    target_link_libraries(AlpineFlags INTERFACE ws2_32 iphlpapi bcrypt)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(AlpineFlags INTERFACE rt dl)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_link_libraries(AlpineFlags INTERFACE dl)
elseif(CMAKE_SYSTEM_NAME MATCHES ".*BSD|DragonFly")
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

# AlpineProtocol and AlpineTransport have mutual symbol references
# (protocol handlers live in AlpineProtocol but are invoked from transport
# dispatch code in AlpineTransport, and vice versa).  GNU ld is a single-pass
# linker that cannot resolve a static-library cycle without --start-group /
# --end-group.  We wrap both archives in a CMake LINK_GROUP:RESCAN helper and
# expose it as Alpine::Stack, which every downstream binary and test should
# link instead of the two libraries directly.
add_library(AlpineStack INTERFACE)
add_library(Alpine::Stack ALIAS AlpineStack)
target_link_libraries(AlpineStack INTERFACE
    "$<LINK_GROUP:RESCAN,Alpine::AlpineProtocol,Alpine::AlpineTransport>"
)

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
    add_subdirectory(test/integration)
    add_subdirectory(test/load)
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

# Sample configuration installed under ${sysconfdir}/alpine/ — packagers can
# wire this as a config file via dpkg/rpm conffiles.
install(FILES packaging/etc/alpine/alpine.conf
    DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/alpine
    COMPONENT runtime
)

# pkg-config (.pc) file so downstream C++ projects can `pkg-config --cflags alpine`.
configure_file(
    ${CMAKE_SOURCE_DIR}/packaging/pkgconfig/alpine.pc.in
    ${CMAKE_BINARY_DIR}/alpine.pc
    @ONLY
)
install(FILES ${CMAKE_BINARY_DIR}/alpine.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
    COMPONENT development
)

# systemd unit — only installed on Linux and only when explicitly requested,
# because not every packager wants us to drop files into /lib/systemd/system/.
if(ALPINE_INSTALL_SYSTEMD AND CMAKE_SYSTEM_NAME STREQUAL "Linux")
    install(FILES packaging/systemd/alpine-rest-bridge.service
        DESTINATION lib/systemd/system
        COMPONENT runtime
    )
endif()

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
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/sonoransun/alpine")
# Base runtime dependencies; SHLIBDEPS fills in the libc6/libstdc++6/libssl3 lines
# automatically, so this is just a floor for packages that link optional features.
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.38), libstdc++6 (>= 14), zlib1g")
if(ALPINE_ENABLE_TLS)
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libssl3")
endif()
if(ALPINE_ENABLE_FUSE)
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libfuse3-3")
endif()
set(CPACK_DEBIAN_PACKAGE_RECOMMENDS "tini")

# RPM settings
set(CPACK_RPM_PACKAGE_LICENSE "LGPL")
set(CPACK_RPM_PACKAGE_GROUP "Applications/Internet")
set(CPACK_RPM_PACKAGE_URL "https://github.com/sonoransun/alpine")
set(CPACK_RPM_PACKAGE_REQUIRES "glibc >= 2.38, libstdc++ >= 14, zlib")
if(ALPINE_ENABLE_TLS)
    set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, openssl-libs")
endif()
if(ALPINE_ENABLE_FUSE)
    set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, fuse3-libs")
endif()

include(CPack)
