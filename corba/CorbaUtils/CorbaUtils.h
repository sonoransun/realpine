/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
////
// ACE+TAO specific
//
#include <orbsvcs/CosNamingC.h>
#include <orbsvcs/Naming/Naming_Utils.h>
#include <tao/corba.h>
//#include <tao/TAO.h>


// The ACE exception handling work around is visually annoying (to me), and may
// be removed at some point.

#define ExTryEnv                          ACE_TRY_ENV
#define ExAny                             ACE_ANY_EXCEPTION
#define ExNewEnv                          ACE_DECLARE_NEW_CORBA_ENV
#define ExAdoptEnv(ENV)                   ACE_ADOPT_CORBA_ENV(ENV)
#define ExCheck                           ACE_CHECK
#define ExCheckReturn(RETVAL)             ACE_CHECK_RETURN(RETVAL)
#define ExThrow(EXCEPTION)                ACE_THROW(EXCEPTION)
#define ExThrowReturn(EXCEPTION,RETVAL)   ACE_THROW_RETURN(EXCEPTION,RETVAL)
#define ExThrowSpec(X)                    ACE_THROW_SPEC(X)
#define ExTry                             ACE_TRY
#define ExTryFor(NAME)                    ACE_TRY_EX(NAME)
#define ExTryCheck                        ACE_TRY_CHECK
#define ExTryCheckFor(NAME)               ACE_TRY_CHECK_EX(NAME)
#define ExTryThrow(EXCEPTION)             ACE_TRY_THROW(EXCEPTION)
#define ExTryThrowFor(EXCEPTION,NAME)     ACE_TRY_THROW_EX(EXCEPTION,NAME)
#define ExCatch(NAME,VAR)                 ACE_CATCH(NAME,VAR)
#define ExCatchAny                        ACE_CATCHANY
#define ExCatchAll                        ACE_CATCHALL
#define ExRethrow                         ACE_RETHROW
#define ExEndTry                          ACE_ENDTRY



