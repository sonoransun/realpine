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
