/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <AlpineBaseOptionData.h>


////
//
// Query Optional Extensions
//
// If you are providing extended options for use in queries, this class must
// be implemented in the derived class providing support for the extensions.
//
// Refer to the AlpineBaseOptionData class for additional interface requirements.
//
// Note that the Option ID used to identify extensions MUST BE UNIQUE across
// all supported options.  Please see the ALPINE development site for information
// on which ID's are in use.
//
// http://cubicmetercrystal.com/alpine/
//
////


class DataBuffer;


class AlpineQueryOptionData : public AlpineBaseOptionData
{
  public:

    AlpineQueryOptionData () = default;

    virtual ~AlpineQueryOptionData ();



    // Copy option data
    //
    virtual AlpineQueryOptionData *  duplicate () = 0;
   

};

