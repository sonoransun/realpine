# FindTAO.cmake
# Locates ACE/TAO CORBA framework via ACE_ROOT and TAO_ROOT environment variables.
# Creates IMPORTED targets: TAO::ACE, TAO::TAO, TAO::CosNaming, TAO::SvcUtils
# Also sets TAO_IDL_COMPILER to the tao_idl path.

if(NOT DEFINED ENV{ACE_ROOT})
    message(FATAL_ERROR "ACE_ROOT environment variable not set")
endif()
if(NOT DEFINED ENV{TAO_ROOT})
    message(FATAL_ERROR "TAO_ROOT environment variable not set")
endif()

set(ACE_ROOT $ENV{ACE_ROOT})
set(TAO_ROOT $ENV{TAO_ROOT})

find_program(TAO_IDL_COMPILER tao_idl PATHS ${TAO_ROOT}/bin NO_DEFAULT_PATH)
if(NOT TAO_IDL_COMPILER)
    message(FATAL_ERROR "tao_idl not found in ${TAO_ROOT}/bin")
endif()

# ACE library
add_library(TAO::ACE SHARED IMPORTED)
set_target_properties(TAO::ACE PROPERTIES
    IMPORTED_LOCATION "${ACE_ROOT}/lib/libACE.so"
    INTERFACE_INCLUDE_DIRECTORIES "${ACE_ROOT}"
    INTERFACE_COMPILE_DEFINITIONS "ACE_HAS_AIO_CALLS;ACE_HAS_EXCEPTIONS"
)

# TAO library
add_library(TAO::TAO SHARED IMPORTED)
set_target_properties(TAO::TAO PROPERTIES
    IMPORTED_LOCATION "${TAO_ROOT}/lib/libTAO.so"
    INTERFACE_INCLUDE_DIRECTORIES "${TAO_ROOT};${TAO_ROOT}/orbsvcs"
    INTERFACE_COMPILE_DEFINITIONS "TAO_ORB"
    INTERFACE_LINK_LIBRARIES "TAO::ACE"
)

# TAO CosNaming
add_library(TAO::CosNaming SHARED IMPORTED)
set_target_properties(TAO::CosNaming PROPERTIES
    IMPORTED_LOCATION "${TAO_ROOT}/lib/libTAO_CosNaming.so"
    INTERFACE_LINK_LIBRARIES "TAO::TAO"
)

# TAO Svc_Utils
add_library(TAO::SvcUtils SHARED IMPORTED)
set_target_properties(TAO::SvcUtils PROPERTIES
    IMPORTED_LOCATION "${TAO_ROOT}/lib/libTAO_Svc_Utils.so"
    INTERFACE_LINK_LIBRARIES "TAO::TAO"
)

# Convenience function to compile IDL files
function(tao_idl_compile IDL_FILE OUTPUT_DIR)
    get_filename_component(IDL_NAME ${IDL_FILE} NAME_WE)
    set(IDL_OUTPUTS
        ${OUTPUT_DIR}/${IDL_NAME}C.cpp
        ${OUTPUT_DIR}/${IDL_NAME}C.h
        ${OUTPUT_DIR}/${IDL_NAME}S.cpp
        ${OUTPUT_DIR}/${IDL_NAME}S.h
        ${OUTPUT_DIR}/${IDL_NAME}C.i
        ${OUTPUT_DIR}/${IDL_NAME}S.i
        ${OUTPUT_DIR}/${IDL_NAME}S_T.cpp
        ${OUTPUT_DIR}/${IDL_NAME}S_T.h
        ${OUTPUT_DIR}/${IDL_NAME}S_T.i
    )
    add_custom_command(
        OUTPUT ${IDL_OUTPUTS}
        COMMAND ${TAO_IDL_COMPILER} -Ge 1 -o ${OUTPUT_DIR} ${IDL_FILE}
        DEPENDS ${IDL_FILE}
        COMMENT "Compiling IDL: ${IDL_FILE}"
    )
    set(TAO_IDL_OUTPUTS ${IDL_OUTPUTS} PARENT_SCOPE)
endfunction()

set(TAO_FOUND TRUE)
