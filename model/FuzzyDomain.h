/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#ifndef __FuzzyDomain_h__
#define __FuzzyDomain_h__


class FuzzyDomain
{

  public:

    FuzzyDomain () {};
    ~FuzzyDomain () {};


    static float SigHalfCurve (const double & value,
                               const double & low,
                               const double & high);

    static float BetaHalfCurve (const double & value,
                                const double & slope,
                                const double & max);

    static float PiCurve (const double & value,
                          const double & midPoint,
                          const double & width);

    static float BetaCurve (const double & value,
                            const double & midPoint,
                            const double & width);
};

#endif
