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

    AlpineBaseOptionData () = default;

    virtual ~AlpineBaseOptionData ();


   
    // Misc optional extension information.
    // 
    virtual ulong  getOptionId () = 0;

    virtual string  getOptionDescription () = 0;


    // Total Data Length for raw packet data
    //
    virtual ulong  getOptionDataLength () = 0;


    // Marshalling option data to/from query packets
    //
    virtual bool  writeData (DataBuffer * linkBuffer) = 0;

    virtual bool  readData (DataBuffer * linkBuffer) = 0;


    // Marshalling option data to/from strings (for use in interface operations)
    //
    virtual bool  writeData (string &  stringBuffer) = 0;

    virtual bool  readData (const string &  stringBuffer) = 0;


};

