add_library(NetUtils
    CovertChannel.cpp
    MulticastDiscovery.cpp
    NetUtils.cpp
    TcpAcceptor.cpp
    TcpAsyncConnector.cpp
    TcpConnector.cpp
    TcpTransport.cpp
    UdpConnection.cpp
    WifiBeaconInjector.cpp
    WifiDiscovery.cpp
)
add_library(Alpine::NetUtils ALIAS NetUtils)
target_include_directories(NetUtils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(NetUtils PUBLIC Alpine::AppUtils Alpine::ThreadUtils)
