add_executable(mapTest
    mapTest.cpp
)
target_include_directories(mapTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(mapTest PRIVATE Alpine::AppUtils Alpine::ThreadUtils)
