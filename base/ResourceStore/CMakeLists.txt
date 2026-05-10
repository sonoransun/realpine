add_library(ResourceStore
    ResourceStore.cpp
)
add_library(Alpine::ResourceStore ALIAS ResourceStore)
target_include_directories(ResourceStore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ResourceStore PUBLIC Alpine::Flags Alpine::AppUtils Alpine::ThreadUtils)
