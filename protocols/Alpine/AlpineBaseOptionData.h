/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>


////
//
// Base Optional Extensions
//
// All optional extention data is derived from this abstract class.  All operations
// specified are required for any extended option support.
//
////


class DataBuffer;


class AlpineBaseOptionData
{
  public:
    AlpineBaseOptionData() = default;

    virtual ~AlpineBaseOptionData();


    // Misc optional extension information.
    //
    virtual ulong getOptionId() = 0;

    virtual string getOptionDescription() = 0;


    // Total Data Length for raw packet data
    //
    virtual ulong getOptionDataLength() = 0;


    // Marshalling option data to/from query packets
    //
    virtual bool writeData(DataBuffer * linkBuffer) = 0;

    virtual bool readData(DataBuffer * linkBuffer) = 0;


    // Marshalling option data to/from strings (for use in interface operations)
    //
    virtual bool writeData(string & stringBuffer) = 0;

    virtual bool readData(const string & stringBuffer) = 0;
};
