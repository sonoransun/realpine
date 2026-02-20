add_executable(tcpServer
    TcpClientThread.cpp
    tcpServer.cpp
    TcpServerConfig.cpp
)
target_include_directories(tcpServer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(tcpServer PRIVATE
    Alpine::ApplCore
    Alpine::AppUtils
    Alpine::NetUtils
    Alpine::ThreadUtils
    Alpine::ConfigUtils
)

add_executable(tcpClient
    TcpClientThread.cpp
    tcpClient.cpp
    TcpClientConfig.cpp
    TcpClientConnector.cpp
)
target_include_directories(tcpClient PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(tcpClient PRIVATE
    Alpine::ApplCore
    Alpine::AppUtils
    Alpine::NetUtils
    Alpine::ThreadUtils
    Alpine::ConfigUtils
)
