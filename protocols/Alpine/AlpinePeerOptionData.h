/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <AlpineBaseOptionData.h>
#include <Common.h>


////
//
// Peer Optional Extensions
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


class AlpinePeerOptionData : public AlpineBaseOptionData
{
  public:
    AlpinePeerOptionData() = default;

    virtual ~AlpinePeerOptionData();


    // Copy option data
    //
    virtual AlpinePeerOptionData * duplicate() = 0;
};
