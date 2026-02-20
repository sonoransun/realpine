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


#include <AlpineCorba_impl.h>
#include <NetUtils.h>
#include <Log.h>
#include <StringUtils.h>



AlpineCorba_impl::AlpineQuery::AlpineQuery (const CORBA::ORB_var &          orb,
                                            const PortableServer::POA_var & poa,
                                            AlpineCorba_impl *              implParent)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::AlpineQuery constructor invoked.");
#endif

    orb_        = orb;
    poa_        = poa;
    implParent_ = implParent;
}



AlpineCorba_impl::AlpineQuery::~AlpineQuery ()
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::AlpineQuery destructor invoked.");
#endif
}



void  
AlpineCorba_impl::AlpineQuery::getDefaultOptions (Alpine::t_QueryOptions_out  options,
                                                  CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_AlpineStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::AlpineQuery::getDefaultOptions invoked.");
#endif

    options = new Alpine::t_QueryOptions;
}



void  
AlpineCorba_impl::AlpineQuery::setDefaultOptions (const Alpine::t_QueryOptions &  options,
                                                  CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_AlpineStackError,
                  Alpine::e_InvalidQueryOptions))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::AlpineQuery::setDefaultOptions invoked.");
#endif
}



void  
AlpineCorba_impl::AlpineQuery::startQuery (const Alpine::t_QueryOptions &  options,
                                           const char *                    queryString,
                                           uint &                          queryId,
                                           CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_AlpineStackError,
                  Alpine::e_InvalidQueryOptions))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQuery::startQuery invoked.");
#endif
}



CORBA::Boolean  
AlpineCorba_impl::AlpineQuery::inProgress (uint  queryId,
                                           CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_InvalidQueryId))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQuery::inProgress invoked.");
#endif


    return 1;
}



void  
AlpineCorba_impl::AlpineQuery::getQueryStatus (uint                       queryId,
                                               Alpine::t_QueryStatus_out  queryStatus,
                                               CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_AlpineStackError,
                  Alpine::e_InvalidQueryId,
                  Alpine::e_InvalidQueryOperation))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQuery::getQueryStatus invoked.");
#endif
}



void  
AlpineCorba_impl::AlpineQuery::pauseQuery (uint  queryId,
                                           CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_AlpineStackError,
                  Alpine::e_InvalidQueryId,
                  Alpine::e_InvalidQueryOperation))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQuery::pauseQuery invoked.");
#endif
}



void  
AlpineCorba_impl::AlpineQuery::resumeQuery (uint  queryId,
                                            CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_AlpineStackError,
                  Alpine::e_InvalidQueryId,
                  Alpine::e_InvalidQueryOperation))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQuery::resumeQuery invoked.");
#endif
}



void  
AlpineCorba_impl::AlpineQuery::cancelQuery (uint  queryId,
                                            CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_AlpineStackError,
                  Alpine::e_InvalidQueryId,
                  Alpine::e_InvalidQueryOperation))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQuery::cancelQuery invoked.");
#endif
}



void  
AlpineCorba_impl::AlpineQuery::getQueryResults (uint                             queryId,
                                                Alpine::t_PeerResourcesList_out  results,
                                                CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_AlpineStackError,
                  Alpine::e_InvalidQueryId,
                  Alpine::e_InvalidQueryOperation))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQuery::getQueryResults invoked.");
#endif

    results = new Alpine::t_PeerResourcesList;
}



void  
AlpineCorba_impl::AlpineQuery::getActiveQueryIdList (Alpine::t_QueryIdList_out  queryIdList,
                                                     CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_AlpineStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::AlpineQuery::getActiveQueryIdList invoked.");
#endif

    queryIdList = new Alpine::t_QueryIdList;
}



void  
AlpineCorba_impl::AlpineQuery::getPastQueryIdList (Alpine::t_QueryIdList_out  queryIdList,
                                                   CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_AlpineStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::AlpineQuery::getPastQueryIdList invoked.");
#endif

    queryIdList = new Alpine::t_QueryIdList;
}



