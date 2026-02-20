add_executable(corbaUtilsTest
    CorbaUtilsTest.cpp
    TestConfig.cpp
)
target_include_directories(corbaUtilsTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(corbaUtilsTest PRIVATE
    Alpine::CorbaUtils
    Alpine::AppUtils
    Alpine::ThreadUtils
    Alpine::ConfigUtils
)
