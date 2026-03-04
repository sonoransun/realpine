set(NETUTILS_SOURCES
    BroadcastUdpConnection.cpp
    CovertChannel.cpp
    InterfaceEnumerator.cpp
    MulticastDiscovery.cpp
    MulticastUdpConnection.cpp
    NetUtils.cpp
    RawWifiConnection.cpp
    TcpAcceptor.cpp
    TcpAsyncConnector.cpp
    TcpConnector.cpp
    TcpTransport.cpp
    TorService.cpp
    TorSocksProxy.cpp
    UdpConnection.cpp
    WifiBeaconInjector.cpp
    WifiDiscovery.cpp
)

if(ALPINE_ENABLE_TLS)
    list(APPEND NETUTILS_SOURCES TlsContext.cpp)
endif()

if(ALPINE_ENABLE_UPNP)
    list(APPEND NETUTILS_SOURCES UpnpPortMapper.cpp)
endif()

add_library(NetUtils ${NETUTILS_SOURCES})
add_library(Alpine::NetUtils ALIAS NetUtils)
target_include_directories(NetUtils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(NetUtils PUBLIC Alpine::AppUtils Alpine::ThreadUtils)

if(ALPINE_ENABLE_TLS)
    target_link_libraries(NetUtils PUBLIC OpenSSL::SSL OpenSSL::Crypto)
    target_compile_definitions(NetUtils PUBLIC ALPINE_TLS_ENABLED)
endif()

if(ALPINE_ENABLE_UPNP)
    target_link_libraries(NetUtils PUBLIC libminiupnpc-static)
    target_compile_definitions(NetUtils PUBLIC ALPINE_UPNP_ENABLED)
endif()
