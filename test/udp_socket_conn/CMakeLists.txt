add_executable(udpSockServer
    udpSockServer.cpp
)
target_include_directories(udpSockServer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(udpSockServer PRIVATE Alpine::AppUtils Alpine::NetUtils Alpine::ThreadUtils)

add_executable(udpSockClient
    udpSockClient.cpp
)
target_include_directories(udpSockClient PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(udpSockClient PRIVATE Alpine::AppUtils Alpine::NetUtils Alpine::ThreadUtils)

# Pair runner: starts the server in the background, runs the client which
# terminates the server cleanly via the documented zero-length packet.
add_test(NAME UdpSock::pair
         COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/run_pair.sh)
set_tests_properties(UdpSock::pair PROPERTIES
    TIMEOUT 30
    ENVIRONMENT "ALPINE_BIN_DIR=${CMAKE_BINARY_DIR}/bin")
