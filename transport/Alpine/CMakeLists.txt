# AlpineTransport and AlpineProtocol have a mutual dependency.
# We break it with INTERFACE header targets: each library gets the other's
# include paths without creating a circular link dependency.
# The final binaries link both libraries together.

add_library(AlpineTransportHeaders INTERFACE)
target_include_directories(AlpineTransportHeaders INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_library(AlpineTransport
    AlpineBroadcastUdpTransport.cpp
    AlpineDtcpConnAcceptor.cpp
    AlpineDtcpConnConnector.cpp
    AlpineDtcpConnMux.cpp
    AlpineDtcpConnTransport.cpp
    AlpineDtcpUdpTransport.cpp
    AlpineMulticastUdpTransport.cpp
    AlpineRawWifiUdpTransport.cpp
)
add_library(Alpine::AlpineTransport ALIAS AlpineTransport)
target_include_directories(AlpineTransport PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(AlpineTransport PUBLIC
    Alpine::Dtcp
    Alpine::AppUtils
    Alpine::NetUtils
    AlpineProtocolHeaders
)
