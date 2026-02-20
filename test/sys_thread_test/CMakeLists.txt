add_executable(sysThreadTest
    TestThread.cpp
    threadTest.cpp
)
target_include_directories(sysThreadTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(sysThreadTest PRIVATE Alpine::AppUtils Alpine::ThreadUtils)
