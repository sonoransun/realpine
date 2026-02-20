add_executable(dtcpStackTestServer
    dtcpStackTestServer.cpp
    DtcpStackTest.cpp
    ServerTestConfig.cpp
)
target_include_directories(dtcpStackTestServer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(dtcpStackTestServer PRIVATE
    Alpine::ApplCore
    Alpine::AlpineProtocol
    Alpine::AlpineTransport
    Alpine::AppUtils
    Alpine::NetUtils
    Alpine::ConfigUtils
)

add_executable(dtcpStackTestClient
    dtcpStackTestClient.cpp
    DtcpStackTest.cpp
    ClientTestConfig.cpp
)
target_include_directories(dtcpStackTestClient PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(dtcpStackTestClient PRIVATE
    Alpine::ApplCore
    Alpine::AlpineProtocol
    Alpine::AlpineTransport
    Alpine::AppUtils
    Alpine::NetUtils
    Alpine::ConfigUtils
)
