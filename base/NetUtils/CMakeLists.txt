add_library(NetUtils
    BroadcastUdpConnection.cpp
    CovertChannel.cpp
    MulticastDiscovery.cpp
    MulticastUdpConnection.cpp
    NetUtils.cpp
    RawWifiConnection.cpp
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
