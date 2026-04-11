add_executable(threadTest
    TestThread.cpp
    threadTest.cpp
)
target_include_directories(threadTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(threadTest PRIVATE Alpine::AppUtils Alpine::ThreadUtils)

# threadTest is a positional-args executable that creates N suspendable threads,
# pauses 10s, resumes them, pauses another 10s and exits cleanly.  Use a small
# thread count and a 30s timeout to keep ctest snappy.
add_test(NAME ThreadTest::auto_basic
         COMMAND threadTest /tmp/alpine-auto-threadtest.log 2 4
         WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set_tests_properties(ThreadTest::auto_basic PROPERTIES TIMEOUT 30)
