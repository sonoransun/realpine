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
