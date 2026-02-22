# Copyright (C) 2026 sonoransun — see LICENCE.txt


# library target
#
LIBRARY = AlpineCorbaClient


# required libs
USE_CORBA             = true
USE_TAO_ORB           = true
USE_LIB_CORBA_UTILS   = true
USE_LIB_ALPINE_IDL    = true
USE_LIB_APP_UTILS     = true
USE_LIB_THREAD_UTILS  = true



# target settings
#
INCLUDES = 
LIBRARIES =

CPP_FILES = \
    AlpineCorbaClient.cpp \
    AlpineGroupMgmt.cpp \
    AlpineModuleMgmt.cpp \
    AlpinePeerMgmt.cpp \
    AlpineQuery.cpp \
    AlpineStackMgmt.cpp \
    AlpineStackStatus.cpp \
    DtcpPeerMgmt.cpp \
    DtcpStackMgmt.cpp \
    DtcpStackStatus.cpp \
    

# build rules
#
include $(ALPINE_ROOT)/build/library.mk

