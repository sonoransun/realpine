# Copyright (C) 2026 sonoransun — see LICENCE.txt


# library target
#
LIBRARY = AlpineTransport


# required libs
#
USE_LIB_ALPINE_PROTOCOL  = true
USE_LIB_DTCP             = true
USE_LIB_APP_UTILS        = true
USE_LIB_NET_UTILS        = true


# target settings
#
INCLUDES = 
LIBRARIES =

CPP_FILES = \
    AlpineDtcpConnAcceptor.cpp \
    AlpineDtcpConnConnector.cpp \
    AlpineDtcpConnMux.cpp \
    AlpineDtcpConnTransport.cpp \
    AlpineDtcpUdpTransport.cpp \




# build rules
#
include $(ALPINE_ROOT)/build/library.mk

