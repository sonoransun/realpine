add_executable(spawnProcess
    spawnProcess.cpp
)
target_include_directories(spawnProcess PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(spawnProcess PRIVATE Alpine::AppUtils Alpine::SysUtils Alpine::ThreadUtils)

# spawnProcess takes positional args:
#   <logFile> <debugLevel 1-4> <command> <quoted args> <createCount>
# It spawns the command `createCount` times via SpawnProcess and exits.
# We use /bin/true so the smoke test costs nothing and is portable.
add_test(NAME SpawnProcess::true_smoke
         COMMAND spawnProcess /tmp/alpine-spawn.log 2 /bin/true "" 2
         WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set_tests_properties(SpawnProcess::true_smoke PROPERTIES TIMEOUT 30)
