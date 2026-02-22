# Copyright (C) 2026 sonoransun — see LICENCE.txt


# library target
#
LIBRARY = Dtcp


# required libs
#
USE_LIB_APP_UTILS     = true
USE_LIB_NET_UTILS     = true
USE_LIB_THREAD_UTILS  = true
USE_LIB_TRANS_BASE    = true


# target settings
#
INCLUDES = 
LIBRARIES =

CPP_FILES = \
    DtcpBaseConnAcceptor.cpp    \
    DtcpBaseConnConnector.cpp   \
    DtcpBaseConnTransport.cpp   \
    DtcpBaseConnMux.cpp         \
    DtcpBaseUdpTransport.cpp    \
    DtcpBroadcast.cpp           \
    DtcpBroadcastMgr.cpp        \
    DtcpBroadcastSet.cpp        \
    DtcpBroadcastStates.cpp     \
    DtcpConnectionMap.cpp       \
    DtcpConnPacket.cpp          \
    DtcpDataPacket.cpp          \
    DtcpFilter.cpp              \
    DtcpIORecord.cpp            \
    DtcpMonitorThread.cpp       \
    DtcpNetId.cpp               \
    DtcpPacket.cpp              \
    DtcpPendingAckMap.cpp       \
    DtcpSendQueue.cpp           \
    DtcpSocketThread.cpp        \
    DtcpStack.cpp               \
    DtcpStackThread.cpp         \
    DtcpThreadTable.cpp         \



# build rules
#
include $(ALPINE_ROOT)/build/library.mk

