# Copyright (C) 2026 sonoransun — see LICENCE.txt


##
## config-apps.mk
## --------------
## Configuration settings for various libraries and applications
##
## use:  grep 'opt>' config-apps.mk
##     for a list of supported options available to client makefiles.
##


# Initialize
#
APP_PHONY    =
APP_DEPS     =
APP_CPPFLAGS =
APP_INCLUDES =
APP_LDLIBS   =
APP_LDPATHS  =
APP_LDFLAGS  =





# opt> USE_LIB_MODULES
# ----------------
#  settings
#

MODULES_ROOT=$(ALPINE_ROOT)/modules

ifeq (true,$(USE_LIB_MODULES))
  CONCURRENT                   = true

  # Modules are a special case.  They only apply to dynamicaly loaded libraries,
  # and they are incorporated into a running server which contains all the required
  # application libraries.
  #

  #APP_CPPFLAGS +=
  APP_INCLUDES += -I$(MODULES_ROOT)/include
  #APP_LDFLAGS  +=
  #APP_LDLIBS   +=
endif

# - - - - - - - -
#






# opt> USE_LIB_CORBA_ADMIN
# ----------------
#  settings
#

CORBA_ADMIN_ROOT=$(ALPINE_ROOT)/corba/CorbaAdmin

ifeq (true,$(USE_LIB_CORBA_ADMIN))
  CONCURRENT                   = true
  USE_CORBA                    = true
  USE_LIB_ALPINE_CORBA_SERVER  = true
  USE_LIB_CORBA_UTILS          = true
  USE_LIB_APP_UTILS            = true
  USE_LIB_THREAD_UTILS         = true

  #APP_CPPFLAGS +=
  APP_INCLUDES += -I$(CORBA_ADMIN_ROOT)
  #APP_LDFLAGS  +=
  APP_LDLIBS   += -lCorbaAdmin$(OPT_EXT)
endif

# - - - - - - - - 
#






# opt> USE_LIB_CORBA_SERVANT
# ----------------
#  settings
#

CORBA_SERVANT_ROOT=$(ALPINE_ROOT)/corba/CorbaServant

ifeq (true,$(USE_LIB_CORBA_SERVANT))
  CONCURRENT                   = true
  USE_CORBA                    = true
  USE_LIB_ALPINE_CORBA_CLIENT  = true
  USE_LIB_CORBA_UTILS          = true
  USE_LIB_APP_UTILS            = true
  USE_LIB_THREAD_UTILS         = true
  USE_LIB_CONFIG_UTILS         = true

  #APP_CPPFLAGS +=
  APP_INCLUDES += -I$(CORBA_SERVANT_ROOT)
  #APP_LDFLAGS  +=
  APP_LDLIBS   += -lCorbaServant$(OPT_EXT)
endif

# - - - - - - - - 
#






# opt> USE_LIB_ALPINE_CORBA_SERVER
# ----------------
#  settings
#

ALPINE_CORBA_SERVER_ROOT=$(ALPINE_ROOT)/corba/interfaces/AlpineCorbaServer

ifeq (true,$(USE_LIB_ALPINE_CORBA_SERVER))
  CONCURRENT                   = true
  USE_CORBA              = true
  USE_LIB_ALPINE_IDL     = true
  USE_LIB_CORBA_UTILS    = true
  USE_LIB_DTCP_INTERFACE = true
  USE_LIB_APP_UTILS      = true
  USE_LIB_THREAD_UTILS   = true
  USE_LIB_THREAD_UTILS   = true

  #APP_CPPFLAGS +=
  APP_INCLUDES += -I$(ALPINE_CORBA_SERVER_ROOT)
  #APP_LDFLAGS  +=
  APP_LDLIBS   += -lAlpineCorbaServer$(OPT_EXT)
endif

# - - - - - - - - 
#






# opt> USE_LIB_ALPINE_CORBA_CLIENT
# ----------------
#  settings
#

ALPINE_CORBA_CLIENT_ROOT=$(ALPINE_ROOT)/corba/interfaces/AlpineCorbaClient

ifeq (true,$(USE_LIB_ALPINE_CORBA_CLIENT))
  CONCURRENT                   = true
  USE_CORBA              = true
  USE_LIB_ALPINE_IDL     = true
  USE_LIB_CORBA_UTILS    = true
  USE_LIB_APP_UTILS      = true
  USE_LIB_THREAD_UTILS   = true

  #APP_CPPFLAGS +=
  APP_INCLUDES += -I$(ALPINE_CORBA_CLIENT_ROOT)
  #APP_LDFLAGS  +=
  APP_LDLIBS   += -lAlpineCorbaClient$(OPT_EXT)
endif

# - - - - - - - - 
#






# opt> USE_LIB_CORBA_UTILS
# ----------------
#  settings
#

CORBA_UTILS_ROOT=$(ALPINE_ROOT)/corba/CorbaUtils

ifeq (true,$(USE_LIB_CORBA_UTILS))
  CONCURRENT                   = true
  USE_CORBA             = true
  USE_LIB_APP_UTILS     = true
  USE_LIB_THREAD_UTILS  = true

  #APP_CPPFLAGS +=
  APP_INCLUDES += -I$(CORBA_UTILS_ROOT)
  #APP_LDFLAGS  +=
  APP_LDLIBS   += -lCorbaUtils$(OPT_EXT)
endif

# - - - - - - - - 
#






# opt> USE_LIB_ALPINE_IDL
# ----------------
#  settings
#

ALPINE_IDL_ROOT=$(ALPINE_ROOT)/corba/idl/AlpineIdl

ifeq (true,$(USE_LIB_ALPINE_IDL))
  CONCURRENT   = true
  USE_CORBA    = true

  #APP_CPPFLAGS +=
  APP_INCLUDES += -I$(ALPINE_IDL_ROOT)
  #APP_LDFLAGS  +=
  APP_LDLIBS   += -lAlpineIdl$(OPT_EXT)
endif

# - - - - - - - - 
#





# opt> USE_LIB_ALPINE_INTERFACE
# ----------------
#  settings
#

ALPINE_INTERFACE_ROOT=$(ALPINE_ROOT)/interfaces/AlpineStackInterface

ifeq (true,$(USE_LIB_ALPINE_INTERFACE))
  CONCURRENT               = true
  USE_LIB_DTCP             = true
  USE_LIB_ALPINE_PROTOCOL  = true
  USE_LIB_APP_UTILS        = true
  USE_LIB_NET_UTILS        = true

  #APP_CPPFLAGS +=
  APP_INCLUDES += -I$(ALPINE_INTERFACE_ROOT)
  #APP_LDFLAGS  +=
  APP_LDLIBS   += -lAlpineStackInterface$(OPT_EXT)
endif

# - - - - - - - -
#






# opt> USE_LIB_DTCP_INTERFACE
# ----------------
#  settings
#

DTCP_INTERFACE_ROOT=$(ALPINE_ROOT)/interfaces/DtcpStackInterface

ifeq (true,$(USE_LIB_DTCP_INTERFACE))
  CONCURRENT        = true
  USE_LIB_DTCP      = true
  USE_LIB_APP_UTILS = true
  USE_LIB_NET_UTILS = true

  #APP_CPPFLAGS +=
  APP_INCLUDES += -I$(DTCP_INTERFACE_ROOT)
  #APP_LDFLAGS  +=
  APP_LDLIBS   += -lDtcpStackInterface$(OPT_EXT)
endif

# - - - - - - - -
#




# Pre trigger for ALPINE PROTOCOL
#
ifeq (true,$(USE_LIB_ALPINE_TRANSPORT))
  USE_LIB_ALPINE_PROTOCOL = true
endif





# opt> USE_LIB_ALPINE_PROTOCOL
# ----------------
#  settings
#

ALPINE_PROTOCOL_ROOT=$(ALPINE_ROOT)/protocols/Alpine

ifneq (AlpineProtocol,$(LIBRARY))
ifeq (true,$(USE_LIB_ALPINE_PROTOCOL))
  CONCURRENT               = true
  USE_LIB_ALPINE_TRANSPORT = true
  USE_LIB_DTCP             = true
  USE_LIB_APP_UTILS        = true
  USE_LIB_NET_UTILS        = true

  #APP_CPPFLAGS +=
  APP_INCLUDES += -I$(ALPINE_PROTOCOL_ROOT)
  #APP_LDFLAGS  +=
  ifneq (AlpineTransport,$(LIBRARY))
    # Only link in protocol if we are not building the alpine transport layer.
    APP_LDLIBS   += -lAlpineProtocol$(OPT_EXT)
  endif
endif
endif

# - - - - - - - - 
#






# opt> USE_LIB_ALPINE_TRANSPORT
# ----------------
#  settings
#

ALPINE_TRANSPORT_ROOT=$(ALPINE_ROOT)/transport/Alpine

ifneq (AlpineTransport,$(LIBRARY))
ifeq (true,$(USE_LIB_ALPINE_TRANSPORT))
  CONCURRENT              = true
  USE_LIB_ALPINE_PROTOCOL = true
  USE_LIB_APP_UTILS       = true
  USE_LIB_NET_UTILS       = true
  USE_LIB_DTCP            = true

  #APP_CPPFLAGS +=
  APP_INCLUDES += -I$(ALPINE_TRANSPORT_ROOT)
  #APP_LDFLAGS  +=
  APP_LDLIBS   += -lAlpineTransport$(OPT_EXT)
endif
endif

# - - - - - - - - 
#






# opt> USE_LIB_DTCP
# ----------------
#  settings
#

DTCP_ROOT=$(ALPINE_ROOT)/transport/Dtcp

ifeq (true,$(USE_LIB_DTCP))
  CONCURRENT           = true
  USE_LIB_APP_UTILS    = true
  USE_LIB_NET_UTILS    = true
  USE_LIB_THREAD_UTILS = true
  USE_LIB_TRANS_BASE   = true

  #APP_CPPFLAGS +=
  APP_INCLUDES += -I$(DTCP_ROOT)
  #APP_LDFLAGS  +=
  APP_LDLIBS   += -lDtcp$(OPT_EXT)
endif

# - - - - - - - - 
#






# opt> USE_LIB_TRANS_BASE
# ----------------
#  settings
#

TRANS_BASE_ROOT=$(ALPINE_ROOT)/transport/TransBase

ifeq (true,$(USE_LIB_TRANS_BASE))
  CONCURRENT           = true
  USE_LIB_APP_UTILS    = true

  #APP_CPPFLAGS +=
  APP_INCLUDES += -I$(TRANS_BASE_ROOT)
  #APP_LDFLAGS  +=
  APP_LDLIBS   += -lTransBase$(OPT_EXT)
endif

# - - - - - - - - 
#






# opt> USE_LIB_APPLCORE
# ----------------
#  settings
#

APPLCORE_BASE_ROOT=$(ALPINE_ROOT)/applcore/ApplCore

ifeq (true,$(USE_LIB_APPLCORE))
  CONCURRENT           = true
  USE_LIB_APP_UTILS    = true
  USE_LIB_NET_UTILS    = true
  USE_LIB_SYS_UTILS    = true
  USE_LIB_THREAD_UTILS = true
  USE_LIB_CONFIG_UTILS = true

  #APP_CPPFLAGS +=
  APP_INCLUDES += -I$(APPLCORE_BASE_ROOT)
  #APP_LDFLAGS  +=
  APP_LDLIBS   += -lApplCore$(OPT_EXT)
endif

# - - - - - - - - 
#






# opt> USE_LIB_CONFIG_UTILS
# ----------------
#  settings
#

CONFIG_UTILS_BASE_ROOT=$(ALPINE_ROOT)/base/ConfigUtils

ifeq (true,$(USE_LIB_CONFIG_UTILS))
  CONCURRENT           = true
  USE_LIB_APP_UTILS    = true
  USE_LIB_THREAD_UTILS = true

  #APP_CPPFLAGS +=
  APP_INCLUDES += -I$(CONFIG_UTILS_BASE_ROOT)
  #APP_LDFLAGS  +=
  APP_LDLIBS   += -lConfigUtils$(OPT_EXT)
endif

# - - - - - - - - 
#






# Pre-hook for AppUtils which may require ThreadUtils
# (If CONCURRENT=true)
#
ifeq (true,$(USE_LIB_APP_UTILS))
  ifeq (true,$(CONCURRENT))
    USE_LIB_THREAD_UTILS = true
  endif
endif






# opt> USE_LIB_THREAD_UTILS
# ----------------
#  settings
#

THREAD_UTILS_ROOT=$(ALPINE_ROOT)/base/ThreadUtils

ifneq (ThreadUtils,$(LIBRARY))
ifeq (true,$(USE_LIB_THREAD_UTILS))
  CONCURRENT        = true
  USE_LIB_APP_UTILS = true
  USE_LIB_SYS_UTILS = true
  
  ifeq (true,$(USE_ACE_THREADS))
    USE_LIB_ACE       = true
  endif
  ifeq (true,$(USE_POSIX_THREADS))
    # Nothing to set explictly here...
  endif

  #APP_CPPFLAGS += 
  APP_INCLUDES += -I$(THREAD_UTILS_ROOT)
  #APP_LDFLAGS  += 

  ifneq (SysUtils,$(LIBRARY))
    # Sys utils is second in bootstrap.
    APP_LDLIBS   += -lThreadUtils$(OPT_EXT)
  endif
endif
endif

# - - - - - - - - 
#






# opt> USE_LIB_SYS_UTILS
# ----------------
#  settings
#

SYS_UTILS_ROOT=$(ALPINE_ROOT)/base/SysUtils

ifneq (SysUtils,$(LIBRARY))
ifeq (true,$(USE_LIB_SYS_UTILS))
  CONCURRENT        = true

  ifeq (true,$(USE_ACE_THREADS))
    USE_LIB_ACE       = true
  endif
  ifeq (true,$(USE_POSIX_THREADS))
    # Nothing to set explictly here...
  endif

  USE_LIB_APP_UTILS = true

  #APP_CPPFLAGS += 
  APP_INCLUDES += -I$(SYS_UTILS_ROOT)
  #APP_LDFLAGS  += 
  APP_LDLIBS   += -lSysUtils$(OPT_EXT)
endif
endif

# - - - - - - - - 
#






# opt> USE_LIB_ACE
# ----------------
# ACE + TAO settings
#

ifeq (true,$(USE_LIB_ACE))
  CONCURRENT = true

  APP_CPPFLAGS += -Wpointer-arith -pipe -D_POSIX_THREADS -D_POSIX_THREAD_SAFE_FUNCTIONS \
                  -DACE_HAS_AIO_CALLS -DACE_HAS_EXCEPTIONS

  APP_INCLUDES += -I$(ACE_ROOT)
  APP_LDFLAGS  += -L$(ACE_ROOT)/lib
  APP_LDLIBS   += -lACE
endif

# - - - - - - - - 
#






# opt> USE_LIB_NET_UTILS
# ----------------
#  settings
#

NET_UTILS_ROOT=$(ALPINE_ROOT)/base/NetUtils

ifeq (true,$(USE_LIB_NET_UTILS))
  CONCURRENT        = true
  USE_LIB_APP_UTILS = true

  #APP_CPPFLAGS += 
  APP_INCLUDES += -I$(NET_UTILS_ROOT)
  #APP_LDFLAGS  += 
  APP_LDLIBS   += -lNetUtils$(OPT_EXT)
endif

# - - - - - - - - 
#






# opt> USE_LIB_APP_UTILS
# ----------------
#  settings
#

APP_UTILS_ROOT=$(ALPINE_ROOT)/base/AppUtils

ifneq (AppUtils,$(LIBRARY))
ifeq (true,$(USE_LIB_APP_UTILS))
  #APP_CPPFLAGS += 
  APP_INCLUDES += -I$(APP_UTILS_ROOT)
  #APP_LDFLAGS  +=
  APP_LDLIBS   += -lAppUtils$(OPT_EXT) 
endif
else
  # This is a root lib, no deps.
  APP_LDLIBS   =
endif

# - - - - - - - - 
#



