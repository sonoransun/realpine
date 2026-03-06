# Copyright (C) 2026 sonoransun — see LICENCE.txt
# FindFUSE.cmake
# Platform-aware find module for FUSE filesystem libraries.
# Creates imported target: FUSE::FUSE
# Sets FUSE_FOUND, FUSE_INCLUDE_DIRS, FUSE_LIBRARIES, FUSE_DEFINITIONS

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    # macFUSE (fuse2 API)
    find_path(FUSE_INCLUDE_DIR fuse.h
        PATHS
            /usr/local/include/fuse
            /opt/homebrew/include/fuse
    )
    find_library(FUSE_LIBRARY
        NAMES fuse fuse_ino64
        PATHS
            /usr/local/lib
            /opt/homebrew/lib
    )
    set(FUSE_VERSION_DEFINE FUSE_USE_VERSION=26)

elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(_FUSE3 fuse3)
    endif()

    if(_FUSE3_FOUND)
        set(FUSE_INCLUDE_DIR ${_FUSE3_INCLUDE_DIRS})
        set(FUSE_LIBRARY ${_FUSE3_LIBRARIES})
        set(FUSE_VERSION_DEFINE FUSE_USE_VERSION=35)
    else()
        find_path(FUSE_INCLUDE_DIR fuse3/fuse.h)
        find_library(FUSE_LIBRARY NAMES fuse3)
        set(FUSE_VERSION_DEFINE FUSE_USE_VERSION=35)
    endif()

elseif(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(_FUSE fuse)
    endif()

    if(_FUSE_FOUND)
        set(FUSE_INCLUDE_DIR ${_FUSE_INCLUDE_DIRS})
        set(FUSE_LIBRARY ${_FUSE_LIBRARIES})
        set(FUSE_VERSION_DEFINE FUSE_USE_VERSION=26)
    else()
        find_path(FUSE_INCLUDE_DIR fuse.h PATHS /usr/local/include)
        find_library(FUSE_LIBRARY NAMES fuse PATHS /usr/local/lib)
        set(FUSE_VERSION_DEFINE FUSE_USE_VERSION=26)
    endif()

else()
    message(FATAL_ERROR "FUSE is not supported on ${CMAKE_SYSTEM_NAME}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FUSE DEFAULT_MSG FUSE_LIBRARY FUSE_INCLUDE_DIR)

if(FUSE_FOUND)
    set(FUSE_INCLUDE_DIRS ${FUSE_INCLUDE_DIR})
    set(FUSE_LIBRARIES ${FUSE_LIBRARY})
    set(FUSE_DEFINITIONS -D${FUSE_VERSION_DEFINE})

    if(NOT TARGET FUSE::FUSE)
        add_library(FUSE::FUSE UNKNOWN IMPORTED)
        set_target_properties(FUSE::FUSE PROPERTIES
            IMPORTED_LOCATION "${FUSE_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${FUSE_INCLUDE_DIRS}"
            INTERFACE_COMPILE_DEFINITIONS "${FUSE_VERSION_DEFINE}"
        )
    endif()
endif()

mark_as_advanced(FUSE_INCLUDE_DIR FUSE_LIBRARY)
