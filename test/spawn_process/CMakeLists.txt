add_executable(spawnProcess
    spawnProcess.cpp
)
target_include_directories(spawnProcess PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(spawnProcess PRIVATE Alpine::AppUtils Alpine::SysUtils Alpine::ThreadUtils)
