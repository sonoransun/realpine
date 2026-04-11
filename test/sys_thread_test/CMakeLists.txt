add_executable(sysThreadTest
    TestThread.cpp
    threadTest.cpp
)
target_include_directories(sysThreadTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(sysThreadTest PRIVATE Alpine::AppUtils Alpine::ThreadUtils)

# NOT registered with ctest: threadTest.cpp ends with `sleep(3600)` in the
# main thread after spawning the worker threads, so the binary never returns
# of its own accord.  Run it manually with:
#   ./bin/sysThreadTest /tmp/sys.log 2 4
# and ^C when done.
