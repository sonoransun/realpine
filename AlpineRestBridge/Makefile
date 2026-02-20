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


# binary target
#

# required libs
#
USE_LIB_APPLCORE             = true
USE_LIB_ALPINE_INTERFACE     = true
USE_LIB_DTCP_INTERFACE       = true
USE_LIB_APP_UTILS            = true
USE_LIB_NET_UTILS            = true
USE_LIB_THREAD_UTILS         = true
USE_LIB_CONFIG_UTILS         = true



INCLUDES  =
LIBRARIES =
BINARY    = AlpineRestBridge

CPP_FILES = \
   JsonWriter.cpp \
   JsonReader.cpp \
   XmlWriter.cpp \
   HttpRequest.cpp \
   HttpResponse.cpp \
   HttpRouter.cpp \
   HttpServer.cpp \
   RestBridgeConfig.cpp \
   DiscoveryBeacon.cpp \
   BroadcastQueryHandler.cpp \
   TorHiddenService.cpp \
   TorTunnel.cpp \
   ContentStore.cpp \
   DlnaHandler.cpp \
   MediaStreamer.cpp \
   DlnaServer.cpp \
   SsdpService.cpp \
   MdnsService.cpp \
   QueryHandler.cpp \
   PeerHandler.cpp \
   StatusHandler.cpp \
   AlpineRestBridge.cpp \


# build rules
#
include $(ALPINE_ROOT)/build/binary.mk
