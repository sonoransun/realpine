add_library(Dtcp
    DtcpBaseConnAcceptor.cpp
    DtcpBaseConnConnector.cpp
    DtcpBaseConnMux.cpp
    DtcpBaseConnTransport.cpp
    DtcpBaseUdpTransport.cpp
    DtcpBroadcast.cpp
    DtcpBroadcastMgr.cpp
    DtcpBroadcastSet.cpp
    DtcpBroadcastStates.cpp
    DtcpConnectionMap.cpp
    DtcpConnPacket.cpp
    DtcpDataPacket.cpp
    DtcpFilter.cpp
    DtcpIORecord.cpp
    DtcpMonitorThread.cpp
    DtcpNetId.cpp
    DtcpPacket.cpp
    DtcpPendingAckMap.cpp
    DtcpSendQueue.cpp
    DtcpSocketThread.cpp
    DtcpStack.cpp
    DtcpStackThread.cpp
    DtcpThreadTable.cpp
)
add_library(Alpine::Dtcp ALIAS Dtcp)
target_include_directories(Dtcp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(Dtcp PUBLIC
    Alpine::AppUtils
    Alpine::NetUtils
    Alpine::ThreadUtils
    Alpine::TransBase
)
