///////
///
///  Copyright (C) 2026  sonoransun
///
///  Permission is hereby granted, free of charge, to any person obtaining a copy
///  of this software and associated documentation files (the "Software"), to deal
///  in the Software without restriction, including without limitation the rights
///  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
///  copies of the Software, and to permit persons to whom the Software is
///  furnished to do so, subject to the following conditions:
///
///  The above copyright notice and this permission notice shall be included in all
///  copies or substantial portions of the Software.
///
///  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
///  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
///  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
///  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
///  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
///  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
///  SOFTWARE.
///
///////


#include <AlpineCorbaClient.h>
#include <ReadLock.h>
#include <WriteLock.h>
#include <Log.h>
#include <StringUtils.h>



CORBA::ORB_var                     AlpineCorbaClient::orb_s;
bool                               AlpineCorbaClient::initialized_s = false;

Alpine::DtcpPeerMgmt_var           AlpineCorbaClient::dtcpPeerMgmtRef_s;
Alpine::DtcpStackMgmt_var          AlpineCorbaClient::dtcpStackMgmtRef_s;
Alpine::DtcpStackStatus_var        AlpineCorbaClient::dtcpStackStatusRef_s;
Alpine::AlpineStackMgmt_var        AlpineCorbaClient::alpineStackMgmtRef_s;
Alpine::AlpineStackStatus_var      AlpineCorbaClient::alpineStackStatusRef_s;
Alpine::AlpineGroupMgmt_var        AlpineCorbaClient::alpineGroupMgmtRef_s;
Alpine::AlpinePeerMgmt_var         AlpineCorbaClient::alpinePeerMgmtRef_s;
Alpine::AlpineQuery_var            AlpineCorbaClient::alpineQueryRef_s;
Alpine::AlpineModuleMgmt_var       AlpineCorbaClient::alpineModuleMgmtRef_s;

ReadWriteSem                       AlpineCorbaClient::dataLock_s;



bool  
AlpineCorbaClient::initialize (const CORBA::ORB_var &                 orb,
                               const Alpine::DtcpPeerMgmt_var &       dtcpPeerMgmt,
                               const Alpine::DtcpStackMgmt_var &      dtcpStackMgmt,
                               const Alpine::DtcpStackStatus_var &    dtcpStackStatus,
                               const Alpine::AlpineStackMgmt_var &    alpineStackMgmt,
                               const Alpine::AlpineStackStatus_var &  alpineStackStatus,
                               const Alpine::AlpineGroupMgmt_var &    alpineGroupMgmt,
                               const Alpine::AlpinePeerMgmt_var &     alpinePeerMgmt,
                               const Alpine::AlpineQuery_var &        alpineQuery,
                               const Alpine::AlpineModuleMgmt_var &   alpineModuleMgmt)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaClient::initialize invoked.");
#endif

    WriteLock  lock(dataLock_s);

    if (initialized_s) {
        Log::Error ("Attempt to reinitialize AlpineCorbaClient!");
        return false;
    }

    if ( CORBA::is_nil (dtcpPeerMgmt.in()) ||
         CORBA::is_nil (dtcpStackMgmt.in()) ||
         CORBA::is_nil (dtcpStackStatus.in()) ||
         CORBA::is_nil (alpineStackMgmt.in()) ||
         CORBA::is_nil (alpineStackStatus.in()) ||
         CORBA::is_nil (alpineGroupMgmt.in()) ||
         CORBA::is_nil (alpinePeerMgmt.in()) ||
         CORBA::is_nil (alpineQuery.in()) ||
         CORBA::is_nil (alpineModuleMgmt.in())     ) {

        Log::Error ("NIL reference passed to AlpineCorbaClient::initialize!");
        return false;
    }

    orb_s = orb;

    dtcpPeerMgmtRef_s       = dtcpPeerMgmt;
    dtcpStackMgmtRef_s      = dtcpStackMgmt;
    dtcpStackStatusRef_s    = dtcpStackStatus;
    alpineStackMgmtRef_s    = alpineStackMgmt;
    alpineStackStatusRef_s  = alpineStackStatus;
    alpineGroupMgmtRef_s    = alpineGroupMgmt;
    alpinePeerMgmtRef_s     = alpinePeerMgmt;
    alpineQueryRef_s        = alpineQuery;
    alpineModuleMgmtRef_s   = alpineModuleMgmt;

    initialized_s = true;


    return true;
}



