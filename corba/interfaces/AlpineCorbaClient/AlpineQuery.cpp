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



void  
AlpineCorbaClient::AlpineQuery::getDefaultOptions (Alpine::t_QueryOptions_out  options,
                                                   CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_AlpineStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaClient::AlpineQuery::getDefaultOptions invoked.");
#endif

    ReadLock  lock(AlpineCorbaClient::dataLock_s);

    if (AlpineCorbaClient::!initialized_s) {
        Log::Error ("AlpineQuery method invoked before AlpineCorbaClient initialization!");
        ExThrow (CORBA::BAD_INV_ORDER ());
        return;
    }

    AlpineCorbaClient::alpineQueryRef_s->getDefaultOptions (options, ExTryEnv);
}



void  
AlpineCorbaClient::AlpineQuery::setDefaultOptions (const Alpine::t_QueryOptions &  options,
                                                   CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_AlpineStackError,
                  Alpine::e_InvalidQueryOptions))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaClient::AlpineQuery::setDefaultOptions invoked.");
#endif

    ReadLock  lock(AlpineCorbaClient::dataLock_s);

    if (AlpineCorbaClient::!initialized_s) {
        Log::Error ("AlpineQuery method invoked before AlpineCorbaClient initialization!");
        ExThrow (CORBA::BAD_INV_ORDER ());
        return;
    }

    AlpineCorbaClient::alpineQueryRef_s->setDefaultOptions (options, ExTryEnv);
}



void  
AlpineCorbaClient::AlpineQuery::startQuery (const Alpine::t_QueryOptions &  options,
                                            const char *                    queryString,
                                            uint &                          queryId,
                                            CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_AlpineStackError,
                  Alpine::e_InvalidQueryOptions))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaClient::AlpineQuery::startQuery invoked.");
#endif

    ReadLock  lock(AlpineCorbaClient::dataLock_s);

    if (AlpineCorbaClient::!initialized_s) {
        Log::Error ("AlpineQuery method invoked before AlpineCorbaClient initialization!");
        ExThrow (CORBA::BAD_INV_ORDER ());
        return;
    }

    AlpineCorbaClient::alpineQueryRef_s->startQuery (options, queryString, queryId, ExTryEnv);
}



CORBA::Boolean  
AlpineCorbaClient::AlpineQuery::inProgress (uint  queryId,
                                            CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_InvalidQueryId))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaClient::AlpineQuery::inProgress invoked.");
#endif

    ReadLock  lock(AlpineCorbaClient::dataLock_s);

    if (AlpineCorbaClient::!initialized_s) {
        Log::Error ("AlpineQuery method invoked before AlpineCorbaClient initialization!");
        ExThrow (CORBA::BAD_INV_ORDER ());
        return;
    }

    CORBA::Boolean  retVal;
    retVal = AlpineCorbaClient::alpineQueryRef_s->inProgress (queryId, ExTryEnv);


    return retVal;
}



void  
AlpineCorbaClient::AlpineQuery::getQueryStatus (uint                       queryId,
                                                Alpine::t_QueryStatus_out  status,
                                                CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_AlpineStackError,
                  Alpine::e_InvalidQueryId,
                  Alpine::e_InvalidQueryOperation))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaClient::AlpineQuery::getQueryStatus invoked.");
#endif

    ReadLock  lock(AlpineCorbaClient::dataLock_s);

    if (AlpineCorbaClient::!initialized_s) {
        Log::Error ("AlpineQuery method invoked before AlpineCorbaClient initialization!");
        ExThrow (CORBA::BAD_INV_ORDER ());
        return;
    }

    AlpineCorbaClient::alpineQueryRef_s->getQueryStatus (queryId, status, ExTryEnv);
}



void  
AlpineCorbaClient::AlpineQuery::pauseQuery (uint  queryId,
                                            CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_AlpineStackError,
                  Alpine::e_InvalidQueryId,
                  Alpine::e_InvalidQueryOperation))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaClient::AlpineQuery::pauseQuery invoked.");
#endif

    ReadLock  lock(AlpineCorbaClient::dataLock_s);

    if (AlpineCorbaClient::!initialized_s) {
        Log::Error ("AlpineQuery method invoked before AlpineCorbaClient initialization!");
        ExThrow (CORBA::BAD_INV_ORDER ());
        return;
    }

    AlpineCorbaClient::alpineQueryRef_s->pauseQuery (queryId, ExTryEnv);
}



void  
AlpineCorbaClient::AlpineQuery::resumeQuery (uint  queryId,
                                             CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_AlpineStackError,
                  Alpine::e_InvalidQueryId,
                  Alpine::e_InvalidQueryOperation))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaClient::AlpineQuery::resumeQuery invoked.");
#endif

    ReadLock  lock(AlpineCorbaClient::dataLock_s);

    if (AlpineCorbaClient::!initialized_s) {
        Log::Error ("AlpineQuery method invoked before AlpineCorbaClient initialization!");
        ExThrow (CORBA::BAD_INV_ORDER ());
        return;
    }

    AlpineCorbaClient::alpineQueryRef_s->resumeQuery (queryId, ExTryEnv);
}



void  
AlpineCorbaClient::AlpineQuery::cancelQuery (uint  queryId,
                                             CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_AlpineStackError,
                  Alpine::e_InvalidQueryId,
                  Alpine::e_InvalidQueryOperation))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaClient::AlpineQuery::cancelQuery invoked.");
#endif

    ReadLock  lock(AlpineCorbaClient::dataLock_s);

    if (AlpineCorbaClient::!initialized_s) {
        Log::Error ("AlpineQuery method invoked before AlpineCorbaClient initialization!");
        ExThrow (CORBA::BAD_INV_ORDER ());
        return;
    }

    AlpineCorbaClient::alpineQueryRef_s->cancelQuery (queryId, ExTryEnv);
}



void  
AlpineCorbaClient::AlpineQuery::getQueryResults (uint                             queryId,
                                                 Alpine::t_PeerResourcesList_out  results,
                                                 CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_AlpineStackError,
                  Alpine::e_InvalidQueryId,
                  Alpine::e_InvalidQueryOperation))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaClient::AlpineQuery::getQueryResults invoked.");
#endif

    ReadLock  lock(AlpineCorbaClient::dataLock_s);

    if (AlpineCorbaClient::!initialized_s) {
        Log::Error ("AlpineQuery method invoked before AlpineCorbaClient initialization!");
        ExThrow (CORBA::BAD_INV_ORDER ());
        return;
    }

    AlpineCorbaClient::alpineQueryRef_s->getQueryResults (queryId, results, ExTryEnv);
}



void  
AlpineCorbaClient::AlpineQuery::getActiveQueryIdList (Alpine::t_QueryIdList_out  queryIdList,
                                                      CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_AlpineStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaClient::AlpineQuery::getActiveQueryIdList invoked.");
#endif

    ReadLock  lock(AlpineCorbaClient::dataLock_s);

    if (AlpineCorbaClient::!initialized_s) {
        Log::Error ("AlpineQuery method invoked before AlpineCorbaClient initialization!");
        ExThrow (CORBA::BAD_INV_ORDER ());
        return;
    }

    AlpineCorbaClient::alpineQueryRef_s->getActiveQueryIdList (queryIdList, ExTryEnv);
}



void  
AlpineCorbaClient::AlpineQuery::getPastQueryIdList (Alpine::t_QueryIdList_out  queryIdList,
                                                    CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_AlpineStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaClient::AlpineQuery::getPastQueryIdList invoked.");
#endif

    ReadLock  lock(AlpineCorbaClient::dataLock_s);

    if (AlpineCorbaClient::!initialized_s) {
        Log::Error ("AlpineQuery method invoked before AlpineCorbaClient initialization!");
        ExThrow (CORBA::BAD_INV_ORDER ());
        return;
    }

    AlpineCorbaClient::alpineQueryRef_s->getPastQueryIdList (queryIdList, ExTryEnv);
}



