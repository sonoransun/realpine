# Copyright (C) 2026 sonoransun — see LICENCE.txt


# library target
#
LIBRARY = CorbaServant


# required libs
USE_CORBA                    = true
USE_TAO_ORB                  = true
USE_LIB_CORBA_UTILS          = true
USE_LIB_ALPINE_CORBA_CLIENT  = true
USE_LIB_APP_UTILS            = true
USE_LIB_THREAD_UTILS         = true
USE_LIB_CONFIG_UTILS         = true



# target settings
#
INCLUDES = 
LIBRARIES =

CPP_FILES = \
    AlpineGroupMgmtIntf.cpp \
    AlpineModuleMgmtIntf.cpp \
    AlpinePeerMgmtIntf.cpp \
    AlpineQueryIntf.cpp \
    AlpineStackMgmtIntf.cpp \
    AlpineStackStatusIntf.cpp \
    CorbaServant.cpp \
    DtcpPeerMgmtIntf.cpp \
    DtcpStackMgmtIntf.cpp \
    DtcpStackStatusIntf.cpp \
    

# build rules
#
include $(ALPINE_ROOT)/build/library.mk

