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


#include <AlpineRatingEngine.h>
#include <AlpinePeerProfile.h>
#include <Log.h>
#include <StringUtils.h>



// Ctor defaulted in header


// Dtor defaulted in header



bool  
AlpineRatingEngine::queryResponseEvent (AlpinePeerProfile *  profile,
                                        short &              qualityDelta)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineRatingEngine::queryResponseEvent invoked.");
#endif


    return true;
}



bool  
AlpineRatingEngine::badPacketEvent (AlpinePeerProfile *  profile,
                                    short &              qualityDelta)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineRatingEngine::badPacketEvent invoked.");
#endif


    return true;
}



bool  
AlpineRatingEngine::transferFailureEvent (AlpinePeerProfile *  profile,
                                          short &              qualityDelta)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineRatingEngine::transferFailureEvent invoked.");
#endif


    return true;
}



bool  
AlpineRatingEngine::naResourceEvent (AlpinePeerProfile *  profile,
                                     short &              qualityDelta)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineRatingEngine::naResourceEvent invoked.");
#endif


    return true;
}



bool  
AlpineRatingEngine::deceptiveResourceEvent (AlpinePeerProfile *  profile,
                                            short &              qualityDelta)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineRatingEngine::deceptiveResourceEvent invoked.");
#endif


    return true;
}



bool  
AlpineRatingEngine::clientResourceEvaluation (AlpinePeerProfile *  profile,
                                              t_ResourceRating     rating,
                                              short &              qualityDelta)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineRatingEngine::clientResourceEvaluation invoked.");
#endif


    return true;
}



