#//////
#//
#//  Copyright (C) 2026  sonoransun
#//
#//  Permission is hereby granted, free of charge, to any person obtaining a copy
#//  of this software and associated documentation files (the "Software"), to deal
#//  in the Software without restriction, including without limitation the rights
#//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#//  copies of the Software, and to permit persons to whom the Software is
#//  furnished to do so, subject to the following conditions:
#//
#//  The above copyright notice and this permission notice shall be included in all
#//  copies or substantial portions of the Software.
#//
#//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
#//  SOFTWARE.
#//
#//////


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
USE_LIB_APP_UTILS = true
USE_LIB_NET_UTILS = true
USE_LIB_THREAD_UTILS = true


# target settings
#
ifeq (SERVER,${TARGET_ROOT})
  TARGET_FILES = udpSockServer.cpp
  BINARY = udpSockServer
else
  TARGET_FILES = udpSockClient.cpp
  BINARY = udpSockClient
endif

COMMON_FILES =

INCLUDES =
LIBRARIES =
CPP_FILES = $(COMMON_FILES) $(TARGET_FILES)




# build rules
#
include $(ALPINE_ROOT)/build/binary.mk

endif
