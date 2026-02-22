# Copyright (C) 2026 sonoransun — see LICENCE.txt



#
# Options
#

# Threading options
#

# MRP_TEMP !!! TEMP  - for testing, always use posix threads
USE_POSIX_THREADS = true
#USE_ACE_THREADS = true





# Initialize
#
CXX            =
LD             =
AR             =
CPPFLAGS       =
ALL_INCLUDES   =
CPPLIBS        =
CPPLDFLAGS     =

# default
CPP_EXT        = cpp



# Include application / library configuration settings
#
include $(ALPINE_ROOT)/build/config-apps.mk

ifeq (true,$(USE_CORBA))
  include $(ALPINE_ROOT)/build/config-corba.mk
endif



#
# Alpine prereqs   
# 
ALPINE_CPPFLAGS   = 
ALPINE_INCLUDES   = -I$(ALPINE_ROOT)/base
ALPINE_LDPATHS    = -L$(ALPINE_ROOT)/lib
ALPINE_LDFLAGS    =
ALPINE_LDLIBS     =
ALPINE_PHONY      =


#
# System settings
#
SYS_COMPILER      = g++
SYS_ARCHIVER      = ar rsuv 
SYS_CPPLIBS       = -lstdc++ -lrt -ldl
SYS_CPPFLAGS      = -Wall -O3 -fPIC -D_GNU_SOURCE
SYS_INCLUDES      = -I.
SYS_LINKER        = g++ -shared
SYS_LIBTOOL       = libtool
SYS_LIBTOOL_FLAGS = --mode=link
SYS_LIB_LDFLAGS   = -fPIC
SYS_LDFLAGS       =
SYS_LDLIBS        =
SYS_OBJ_DIR       = .obj
SYS_PHONY         =
SYS_DEPS          = 



################################################# MRP_TEMP !!! TEMP
SYS_CPPFLAGS += -D_VERBOSE
################################################# MRP_TEMP !!! TEMP

ifeq (true,$(USE_POSIX_THREADS))
  SYS_CPPFLAGS += -D_POSIX_THREADS
  SYS_CPPLIBS  += -lpthread
endif
ifeq (true,$(USE_ACE_THREADS))
  SYS_CPPFLAGS += -D_ACE_THREADS
endif





# Different build types create libraries and binaries with different extensions.
#
#OPT_EXT=


ifeq (true,$(DEBUG))
  SYS_CPPFLAGS += -g -D_DEBUG -D_VERBOSE
  OPT_EXT :=$(OPT_EXT)-dbg
  NO_REPO = true
endif

ifeq (true,$(PROFILE))
  SYS_CPPFLAGS += -pg -g -D_PROFILE
  DEBUG = true
  OPT_EXT :=$(OPT_EXT)-prof
  NO_REPO = true
endif

ifneq (true,$(DEBUG))
  ifneq (true,$(PROFILE))
    SYS_CPPFLAGS += -fomit-frame-pointer 
  endif
endif


ifeq (true,$(CONCURRENT))
  SYS_CPPFLAGS += -D_CONCURRENT -D_REENTRANT
  OPT_EXT :=$(OPT_EXT)-r
  SYS_LDLIBS += -lmt
endif

ifeq (true,$(USE_STATIC_LIBS))
  SYS_LDFLAGS   += -static
endif

# Sanitizer support (e.g. SANITIZE=address, SANITIZE=thread, SANITIZE=address,undefined)
ifneq (,$(SANITIZE))
  SYS_CPPFLAGS += -fsanitize=$(SANITIZE) -fno-omit-frame-pointer -fno-optimize-sibling-calls
  SYS_LDFLAGS  += -fsanitize=$(SANITIZE)
  SYS_LDLIBS   += -fsanitize=$(SANITIZE)
  OPT_EXT :=$(OPT_EXT)-san
endif



#
# Evaluate
#
BUILD_OBJ_DIR = $(SYS_OBJ_DIR)$(OPT_EXT)$(USER_BUILD_EXT)

OBJECTS = $(addprefix $(BUILD_OBJ_DIR)/, $(subst .$(CPP_EXT),.o,$(CPP_FILES)))

TARGET_DEPS = $(SYS_DEPS) $(CPP_FILES) $(OBJECTS) $(APP_DEPS) $(CORBA_DEPS)

ALL_INCLUDES = $(SYS_INCLUDES) $(APP_INCLUDES) $(CORBA_INCLUDES) $(ALPINE_INCLUDES) $(INCLUDES)
CPPFLAGS = $(SYS_CPPFLAGS) $(ALPINE_CPPFLAGS) $(APP_CPPFLAGS) $(CORBA_CPPFLAGS) $(DEFINES) $(ALL_INCLUDES)

LD = $(SYS_LIBTOOL) $(SYS_LIBTOOL_FLAGS) $(SYS_LINKER)
CXX = $(SYS_COMPILER)
AR = $(SYS_ARCHIVER)


ifeq (true,$(LIBRARY_TARGET))
  LDFLAGS = $(SYS_LIB_LDFLAGS) $(ALPINE_LDFLAGS) $(ALPINE_LDPATHS) $(APP_LDFLAGS) $(CORBA_LDFLAGS)
  CPPLIBS = $(LIBRARIES) $(APP_LDLIBS) $(CORBA_LDLIBS) $(ALPINE_LDLIBS) 
  FULL_LIBRARY = $(LIBRARY)$(OPT_EXT)
else
  LDFLAGS = $(SYS_LDFLAGS) $(ALPINE_LDFLAGS) $(ALPINE_LDPATHS) $(APP_LDFLAGS) $(CORBA_LDFLAGS)
  CPPLIBS = $(SYS_CPPLIBS) $(APP_LDLIBS) $(CORBA_LDLIBS) $(ALPINE_LDLIBS) $(LIBRARIES) 

  FULL_BINARY = $(BINARY)$(OPT_EXT)

  ifeq (true,$(USE_TEMPLATE_REPO))
    SYS_CPPFLAGS      += -frepo
  endif
endif


