add_library(SysUtils
    DynamicLoader.cpp
    FileUtils.cpp
    SignalSet.cpp
    SpawnProcess.cpp
    UserUtils.cpp
)
add_library(Alpine::SysUtils ALIAS SysUtils)
target_include_directories(SysUtils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(SysUtils PUBLIC Alpine::AppUtils)
