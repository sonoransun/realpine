/// Copyright (C) 2026 sonoransun — see LICENCE.txt


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


