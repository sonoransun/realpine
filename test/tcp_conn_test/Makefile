# Copyright (C) 2026 sonoransun — see LICENCE.txt


# binary targets
#
TARGETS = SERVER CLIENT

ifndef TARGET_ROOT 

.PHONY: clean all default $(TARGETS)

all default: $(TARGETS)
	@$(MAKE) -s TARGET_ROOT=SERVER
	@$(MAKE) -s TARGET_ROOT=CLIENT

clean:
	@$(MAKE) -s TARGET_ROOT=SERVER clean
	@$(MAKE) -s TARGET_ROOT=CLIENT clean

else



# required libs
#
USE_LIB_APP_UTILS      = true
USE_LIB_NET_UTILS      = true
USE_LIB_THREAD_UTILS   = true
USE_LIB_APPLCORE       = true
USE_LIB_CONFIG_UTILS   = true


# target settings
#
ifeq (SERVER,${TARGET_ROOT})
  #
  # TCP Server
  #
  BINARY = tcpServer
  TARGET_FILES = \
      tcpServer.cpp \
      TcpServerConfig.cpp

else
  #
  # TCP Client
  #
  BINARY = tcpClient
  TARGET_FILES = \
      tcpClient.cpp \
      TcpClientConfig.cpp \
      TcpClientConnector.cpp

endif


COMMON_FILES = TcpClientThread.cpp


INCLUDES =
LIBRARIES =
CPP_FILES = $(COMMON_FILES) $(TARGET_FILES)




# build rules
#
include $(ALPINE_ROOT)/build/binary.mk

endif
