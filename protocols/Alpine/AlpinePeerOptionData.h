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
#include <AlpineBaseOptionData.h>


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

    AlpinePeerOptionData () = default;

    virtual ~AlpinePeerOptionData ();



    // Copy option data
    //
    virtual AlpinePeerOptionData *  duplicate () = 0;
   

};

