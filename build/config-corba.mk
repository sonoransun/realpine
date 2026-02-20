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


##
## config-corba.mk
## --------------
## Configuration settings for CORBA ORB's and precompilers
##


# Initialize
#
CORBA_PHONY    =
CORBA_DEPS     =
CORBA_CPPFLAGS =
CORBA_INCLUDES =
CORBA_LDLIBS   =
CORBA_LDPATHS  =
CORBA_LDFLAGS  =




# opt> USE_TAO_ORB
# ----------------
#  settings
#


ifeq (true,$(USE_TAO_ORB))

  CONCURRENT=true
  USE_LIB_ACE=true

  CORBA_CPPFLAGS += -D_POSIX_THREADS -D_POSIX_THREAD_SAFE_FUNCTIONS -D_REENTRANT
  CORBA_CPPFLAGS += -DACE_HAS_AIO_CALLS -DACE_HAS_EXCEPTIONS -DTAO_ORB

  CORBA_INCLUDES += -I$(ACE_ROOT) -I$(TAO_ROOT) -I$(TAO_ROOT)/orbsvcs 
  CORBA_LDFLAGS  += -L$(ACE_ROOT)/lib -L$(TAO_ROOT)/lib
  CORBA_LDLIBS   += -lTAO_CosNaming -lTAO_Svc_Utils -lTAO

  IDLFLAGS   = -Ge 1
  IDLCOMPILE = $(TAO_ROOT)/bin/tao_idl

  CORBA_GEN_FILES = $(subst .idl,S.cpp,$(IDL_FILES)) \
                    $(subst .idl,C.cpp,$(IDL_FILES)) \
                    $(subst .idl,S.h,$(IDL_FILES)) \
                    $(subst .idl,C.h,$(IDL_FILES)) \
                    $(subst .idl,S.i,$(IDL_FILES)) \
                    $(subst .idl,C.i,$(IDL_FILES)) \
                    $(subst .idl,S_T.cpp,$(IDL_FILES)) \
                    $(subst .idl,S_T.h,$(IDL_FILES)) \
                    $(subst .idl,S_T.i,$(IDL_FILES)) 

  CPP_FILES += $(subst .idl,S.cpp,$(IDL_FILES)) \
               $(subst .idl,C.cpp,$(IDL_FILES))

  CORBA_DEPS = $(subst .idl,S.cpp,$(IDL_FILES))
endif






# opt> USE_ORBIT_ORB
# ----------------
#  settings
#


ifeq (true,$(USE_ORBIT_ORB))

  ORBIT_RELEASE  = /projects/orbit/release

  CONCURRENT=true

  CORBA_CPPFLAGS += -DORBIT_ORB

  CORBA_INCLUDES += -I$(ORBIT_RELEASE)/include
  CORBA_LDFLAGS  += -L$(ORBIT_RELEASE)/lib
#  CORBA_LDLIBS   += -lTAO_CosNaming -lTAO_Svc_Utils -lTAO

  IDLFLAGS   = -l c++
  IDLCOMPILE = $(ORBIT_RELEASE)/bin/orbit-idl 

  CPP_EXT = cc

  CORBA_GEN_FILES = $(subst .idl,-cpp.cc,$(IDL_FILES)) \
                    $(subst .idl,-cpp.hh,$(IDL_FILES)) \
                    $(subst .idl,-cpp-common.cc,$(IDL_FILES)) \
                    $(subst .idl,-cpp-common.hh,$(IDL_FILES)) \
                    $(subst .idl,-cpp-skels.cc,$(IDL_FILES)) \
                    $(subst .idl,-cpp-skels.hh,$(IDL_FILES)) \
                    $(subst .idl,-cpp-stubs.cc,$(IDL_FILES)) \
                    $(subst .idl,-cpp-stubs.hh,$(IDL_FILES)) \


  CPP_FILES += $(subst .idl,-cpp.cc,$(IDL_FILES)) \
               $(subst .idl,-cpp-common.cc,$(IDL_FILES)) \
               $(subst .idl,-cpp-skels.cc,$(IDL_FILES)) \
               $(subst .idl,-cpp-stubs.cc,$(IDL_FILES)) \


  CORBA_DEPS = $(subst .idl,-cpp.cc,$(IDL_FILES))
endif








