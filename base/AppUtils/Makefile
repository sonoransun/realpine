# Copyright (C) 2026 sonoransun — see LICENCE.txt


# library target
#
LIBRARY = AppUtils


# required libs
CONCURRENT=true
ifeq (true,$(CONCURRENT))
  USE_LIB_THREAD_UTILS = true
endif


# target settings
#
INCLUDES = 
LIBRARIES =

CPP_FILES = \
   AgedQueue.cpp \
   Log.cpp \
   OptHash.cpp \
   PollSet.cpp \
   StringUtils.cpp \



# build rules
#
include $(ALPINE_ROOT)/build/library.mk

