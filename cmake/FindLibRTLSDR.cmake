# FindLibRTLSDR.cmake — Locate librtlsdr (RTL-SDR library)
#
# Sets:
#   LibRTLSDR_FOUND        - TRUE if librtlsdr is available
#   LibRTLSDR_INCLUDE_DIRS - Header search paths
#   LibRTLSDR_LIBRARIES    - Libraries to link
#   LibRTLSDR_VERSION      - Version string (if available)

include(FindPackageHandleStandardArgs)

# Try pkg-config first
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(PC_RTLSDR QUIET librtlsdr)
endif()

find_path(LibRTLSDR_INCLUDE_DIR
    NAMES rtl-sdr.h
    HINTS ${PC_RTLSDR_INCLUDEDIR} ${PC_RTLSDR_INCLUDE_DIRS}
    PATH_SUFFIXES rtlsdr
)

find_library(LibRTLSDR_LIBRARY
    NAMES rtlsdr
    HINTS ${PC_RTLSDR_LIBDIR} ${PC_RTLSDR_LIBRARY_DIRS}
)

if(PC_RTLSDR_VERSION)
    set(LibRTLSDR_VERSION ${PC_RTLSDR_VERSION})
endif()

find_package_handle_standard_args(LibRTLSDR
    REQUIRED_VARS LibRTLSDR_LIBRARY LibRTLSDR_INCLUDE_DIR
    VERSION_VAR LibRTLSDR_VERSION
)

if(LibRTLSDR_FOUND)
    set(LibRTLSDR_LIBRARIES ${LibRTLSDR_LIBRARY})
    set(LibRTLSDR_INCLUDE_DIRS ${LibRTLSDR_INCLUDE_DIR})

    if(NOT TARGET LibRTLSDR::LibRTLSDR)
        add_library(LibRTLSDR::LibRTLSDR UNKNOWN IMPORTED)
        set_target_properties(LibRTLSDR::LibRTLSDR PROPERTIES
            IMPORTED_LOCATION "${LibRTLSDR_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${LibRTLSDR_INCLUDE_DIR}"
        )
    endif()
endif()

mark_as_advanced(LibRTLSDR_INCLUDE_DIR LibRTLSDR_LIBRARY)
