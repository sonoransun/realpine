add_library(ConfigUtils
    ArgumentMap.cpp
    ConfigData.cpp
    ConfigFile.cpp
    ConfigSchema.cpp
    Configuration.cpp
    EnvironMap.cpp
)
add_library(Alpine::ConfigUtils ALIAS ConfigUtils)
target_include_directories(ConfigUtils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ConfigUtils PUBLIC Alpine::AppUtils Alpine::ThreadUtils)
