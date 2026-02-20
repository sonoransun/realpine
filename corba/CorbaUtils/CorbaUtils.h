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



