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


# library target
#
LIBRARY = AlpineCorbaServer


# required libs
USE_CORBA                  = true
USE_TAO_ORB                = true
USE_LIB_CORBA_UTILS        = true
USE_LIB_ALPINE_IDL         = true
USE_LIB_DTCP_INTERFACE     = true
USE_LIB_ALPINE_INTERFACE   = true
USE_LIB_APP_UTILS          = true
USE_LIB_NET_UTILS          = true
USE_LIB_THREAD_UTILS       = true



# target settings
#
INCLUDES = 
LIBRARIES =

CPP_FILES = \
    AlpineCorba_impl.cpp \
    AlpineCorbaServer.cpp \
    AlpineGroupMgmt_impl.cpp \
    AlpineModuleMgmt_impl.cpp \
    AlpinePeerMgmt_impl.cpp \
    AlpineQuery_impl.cpp \
    AlpineStackMgmt_impl.cpp \
    AlpineStackStatus_impl.cpp \
    DtcpPeerMgmt_impl.cpp \
    DtcpStackMgmt_impl.cpp \
    DtcpStackStatus_impl.cpp \
    

# build rules
#
include $(ALPINE_ROOT)/build/library.mk

