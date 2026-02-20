add_library(DynLib SHARED
    DynamicObject.cpp
)
target_include_directories(DynLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(DynLib PRIVATE Alpine::Flags AppUtils ThreadUtils)
