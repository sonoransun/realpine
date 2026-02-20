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


#include <FuzzyDomain.h>
#include <math.h>


float 
FuzzyDomain::SigHalfCurve (const double & value,
                                const double & low,
                                const double & high)
{
    if ( (low >= high) ||
         (value < 0) )
    {
        return -1;
    }

    double midPoint = (high - low)/2 + low;

    if (value >= high) {
        return 1;
    }
    if (value <= low) {
        return 0;
    }

    float retVal;

    if (value <= midPoint) {
        retVal = 2 * pow( ((value - low)/(high - low)), 2);
    }
    else {
        retVal = 1 - 2 * pow( ((value - high)/(high - low)), 2);
    }

    return retVal;
}


float 
FuzzyDomain::BetaHalfCurve (const double & value,
                                 const double & slope,
                                 const double & max)
{
    if (value >= max) {
        return 1.0;
    }

    double denominator;
    denominator = 1 + pow ( ((value - max)/slope), 2);
    double numerator = 1.0;
  
    return numerator / denominator;
}


float 
FuzzyDomain::PiCurve (const double & value,
                           const double & midPoint,
                           const double & width)
{

    return 0.0;
}


float 
FuzzyDomain::BetaCurve (const double & value,
                             const double & midPoint,
                             const double & width)
{

    return 0.0;
}


