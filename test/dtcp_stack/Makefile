# Copyright (C) 2026 sonoransun — see LICENCE.txt


# binary targets
#
TARGETS = SERVER CLIENT

ifndef TARGET_ROOT
# Build all targets

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
USE_LIB_APPLCORE = true
USE_LIB_APP_UTILS = true
USE_LIB_NET_UTILS = true
USE_LIB_ALPINE_TRANSPORT = true
USE_LIB_CONFIG_UTILS = true


# specific target settings
#
ifeq (SERVER,${TARGET_ROOT})
  TARGET_FILES = dtcpStackTestServer.cpp DtcpStackTest.cpp ServerTestConfig.cpp
  BINARY = dtcpStackTestServer
else
  TARGET_FILES = dtcpStackTestClient.cpp DtcpStackTest.cpp ClientTestConfig.cpp
  BINARY = dtcpStackTestClient
endif

COMMON_FILES =
INCLUDES =
LIBRARIES = 
CPP_FILES = $(TARGET_FILES) $(COMMON_FILES)




# build rules
#
include $(ALPINE_ROOT)/build/binary.mk

endif

