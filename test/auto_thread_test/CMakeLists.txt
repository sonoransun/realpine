add_executable(threadTest
    TestThread.cpp
    threadTest.cpp
)
target_include_directories(threadTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(threadTest PRIVATE Alpine::AppUtils Alpine::ThreadUtils)
