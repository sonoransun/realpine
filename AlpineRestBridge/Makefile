# Copyright (C) 2026 sonoransun — see LICENCE.txt


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
