# Copyright (C) 2026 sonoransun — see LICENCE.txt


# library target
#
LIBRARY = AlpineCorbaServer


# required libs
USE_CORBA                  = true
USE_TAO_ORB                = true
USE_LIB_CORBA_UTILS        = true
USE_LIB_ALPINE_IDL         = true
USE_LIB_DTCP_INTERFACE     = true
USE_LIB_ALPINE_INTERFACE   = true
USE_LIB_APP_UTILS          = true
USE_LIB_NET_UTILS          = true
USE_LIB_THREAD_UTILS       = true



# target settings
#
INCLUDES = 
LIBRARIES =

CPP_FILES = \
    AlpineCorba_impl.cpp \
    AlpineCorbaServer.cpp \
    AlpineGroupMgmt_impl.cpp \
    AlpineModuleMgmt_impl.cpp \
    AlpinePeerMgmt_impl.cpp \
    AlpineQuery_impl.cpp \
    AlpineStackMgmt_impl.cpp \
    AlpineStackStatus_impl.cpp \
    DtcpPeerMgmt_impl.cpp \
    DtcpStackMgmt_impl.cpp \
    DtcpStackStatus_impl.cpp \
    

# build rules
#
include $(ALPINE_ROOT)/build/library.mk

