# Copyright (C) 2026 sonoransun — see LICENCE.txt


# binary targets
#
TARGETS = normal lean

ifndef TARGET_ROOT

.PHONY: clean default $(TARGETS)

all default: $(TARGETS)
	@$(MAKE) -s TARGET_ROOT=normal
	@$(MAKE) -s TARGET_ROOT=lean

clean:
	@$(MAKE) -s TARGET_ROOT=normal clean
	@$(MAKE) -s TARGET_ROOT=lean clean

else






# required libraries
#
USE_LIB_APP_UTILS       = true
USE_LIB_ALPINE_PROTOCOL = true
USE_LIB_NET_UTILS       = true
USE_LIB_DTCP            = true


# target settings
#
USER_BUILD_EXT =-$(TARGET_ROOT)

ifeq (normal,${TARGET_ROOT})
  BINARY = memTest
else
  DEFINES = -D_LEAN_N_MEAN
  BINARY  = memTestLean
endif

LIBRARIES =
CPP_FILES = \
   DtcpMemTest.cpp \
   memTest.cpp \




# build rules
#
include $(ALPINE_ROOT)/build/binary.mk

endif
