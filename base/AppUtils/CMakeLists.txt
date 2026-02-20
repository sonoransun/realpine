add_library(AppUtils
    AgedQueue.cpp
    Log.cpp
    OptHash.cpp
    PollSet.cpp
    StringUtils.cpp
)
add_library(Alpine::AppUtils ALIAS AppUtils)
target_include_directories(AppUtils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(AppUtils PUBLIC Alpine::Flags ThreadUtilsHeaders)
