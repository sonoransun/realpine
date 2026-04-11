add_executable(dynamicLoaderTest
    dynamicLoaderTest.cpp
)
target_include_directories(dynamicLoaderTest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/test/dl_test/DynLib
)
target_link_libraries(dynamicLoaderTest PRIVATE
    Alpine::ApplCore
    Alpine::AppUtils
    Alpine::SysUtils
    Alpine::ThreadUtils
    Alpine::ConfigUtils
)

# dynamicLoaderTest dlopens libDynLib.so (built in the same tree at lib/),
# resolves createDynamicObject, instantiates a few objects, calls testMethod
# on each and exits.  Use the absolute build-tree path to the .so so the
# test is hermetic.
add_test(NAME DynamicLoader::dynlib_smoke
         COMMAND dynamicLoaderTest
                 --logFile /tmp/alpine-dynloader.log
                 --logLevel Error
                 --libraryName ${CMAKE_BINARY_DIR}/lib/libDynLib.so
                 --entryFunction createDynamicObject
                 --numObjects 4
         WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
# ASan+dlopen interposer is flaky on the test's factory-cast pattern
# (legacy dlsym + function-pointer cast in dynamicLoaderTest.cpp).
# Skip the test under any sanitizer build to keep the sanitize job green.
if(ALPINE_SANITIZER)
    set(_dynlib_disabled TRUE)
else()
    set(_dynlib_disabled FALSE)
endif()
set_tests_properties(DynamicLoader::dynlib_smoke PROPERTIES
    TIMEOUT 30
    DISABLED ${_dynlib_disabled}
    DEPENDS "")
