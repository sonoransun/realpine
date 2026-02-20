add_library(ApplCore
    ApplCore.cpp
    SignalMonitorThread.cpp
)
add_library(Alpine::ApplCore ALIAS ApplCore)
target_include_directories(ApplCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ApplCore PUBLIC
    Alpine::AppUtils
    Alpine::NetUtils
    Alpine::SysUtils
    Alpine::ThreadUtils
    Alpine::ConfigUtils
)
