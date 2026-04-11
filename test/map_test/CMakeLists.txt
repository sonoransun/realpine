add_executable(mapTest
    mapTest.cpp
)
target_include_directories(mapTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(mapTest PRIVATE Alpine::AppUtils Alpine::ThreadUtils)

# NOT registered with ctest: mapTest.cpp ends with `sleep(3600)` (only skipped
# under _PROFILE builds), so the binary never returns in normal Debug/Release
# builds.  Run it manually with:
#   ./bin/mapTest /tmp/map.log 2 100 64
# and ^C when done — or build with -DCMAKE_BUILD_TYPE=Profile.
