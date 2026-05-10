# Copyright (C) 2026 sonoransun — see LICENCE.txt
#
# Pluggable filesystem-observation backends.  Each backend is gated by an
# ALPINE_ENABLE_FSOBSERVE_* option in the top-level CMakeLists.txt.  When a
# backend is disabled, a stub source is compiled in its place so the
# factory always links — runtime behavior is to log "disabled at build
# time" and return false from start(), letting the factory fall back.
#
# Per-platform default-on selection is decided in the top-level options
# block; this file only consults the resulting flags.

set(FSOBSERVE_SOURCES
    FsObserverFactory.cpp
)

# --- Backend source selection (real vs stub) ---

if(ALPINE_ENABLE_FSOBSERVE_INOTIFY)
    list(APPEND FSOBSERVE_SOURCES InotifyObserver.cpp)
else()
    list(APPEND FSOBSERVE_SOURCES stubs/InotifyObserverStub.cpp)
endif()

if(ALPINE_ENABLE_FSOBSERVE_FANOTIFY)
    list(APPEND FSOBSERVE_SOURCES FanotifyObserver.cpp)
else()
    list(APPEND FSOBSERVE_SOURCES stubs/FanotifyObserverStub.cpp)
endif()

if(ALPINE_ENABLE_FSOBSERVE_AUDIT)
    list(APPEND FSOBSERVE_SOURCES AuditObserver.cpp)
else()
    list(APPEND FSOBSERVE_SOURCES stubs/AuditObserverStub.cpp)
endif()

if(ALPINE_ENABLE_FSOBSERVE_EBPF)
    list(APPEND FSOBSERVE_SOURCES EbpfObserver.cpp)
else()
    list(APPEND FSOBSERVE_SOURCES stubs/EbpfObserverStub.cpp)
endif()

if(ALPINE_ENABLE_FSOBSERVE_KQUEUE)
    list(APPEND FSOBSERVE_SOURCES KqueueObserver.cpp)
else()
    list(APPEND FSOBSERVE_SOURCES stubs/KqueueObserverStub.cpp)
endif()

if(ALPINE_ENABLE_FSOBSERVE_FSEVENTS)
    list(APPEND FSOBSERVE_SOURCES FsEventsObserver.cpp)
else()
    list(APPEND FSOBSERVE_SOURCES stubs/FsEventsObserverStub.cpp)
endif()

if(ALPINE_ENABLE_FSOBSERVE_ENDPOINTSEC)
    list(APPEND FSOBSERVE_SOURCES EndpointSecurityObserver.cpp)
else()
    list(APPEND FSOBSERVE_SOURCES stubs/EndpointSecurityObserverStub.cpp)
endif()

if(ALPINE_ENABLE_FSOBSERVE_READDIRCHANGES)
    list(APPEND FSOBSERVE_SOURCES ReadDirChangesObserver.cpp)
else()
    list(APPEND FSOBSERVE_SOURCES stubs/ReadDirChangesObserverStub.cpp)
endif()

if(ALPINE_ENABLE_FSOBSERVE_ETW)
    list(APPEND FSOBSERVE_SOURCES EtwObserver.cpp)
else()
    list(APPEND FSOBSERVE_SOURCES stubs/EtwObserverStub.cpp)
endif()


add_library(FsObserve ${FSOBSERVE_SOURCES})
add_library(Alpine::FsObserve ALIAS FsObserve)
target_include_directories(FsObserve PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(FsObserve PUBLIC Alpine::Flags Alpine::AppUtils Alpine::ThreadUtils)


# --- Per-backend external library / framework links ---

if(ALPINE_ENABLE_FSOBSERVE_AUDIT)
    find_library(AUDIT_LIBRARY audit REQUIRED)
    target_link_libraries(FsObserve PUBLIC ${AUDIT_LIBRARY})
    target_compile_definitions(FsObserve PRIVATE ALPINE_HAVE_LIBAUDIT=1)
endif()

if(ALPINE_ENABLE_FSOBSERVE_FSEVENTS)
    find_library(CORESERVICES_FRAMEWORK CoreServices REQUIRED)
    target_link_libraries(FsObserve PUBLIC ${CORESERVICES_FRAMEWORK})
endif()

if(ALPINE_ENABLE_FSOBSERVE_ENDPOINTSEC)
    find_library(ENDPOINTSEC_LIB EndpointSecurity REQUIRED)
    find_library(BSM_LIB bsm REQUIRED)
    target_link_libraries(FsObserve PUBLIC ${ENDPOINTSEC_LIB} ${BSM_LIB})
endif()

if(ALPINE_ENABLE_FSOBSERVE_ETW)
    # Tdh.lib provides TdhGetEventInformation / TdhGetProperty.  Some
    # MinGW-w64 distributions don't ship it; the build fails fast in that
    # case, which is preferable to a quiet stub fallback for an opt-in
    # backend the operator explicitly enabled.
    find_library(TDH_LIBRARY tdh REQUIRED)
    target_link_libraries(FsObserve PUBLIC ${TDH_LIBRARY})
endif()
