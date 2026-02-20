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


class AlpineStackConfig
{
  public:

    AlpineStackConfig ();

    AlpineStackConfig (const AlpineStackConfig & copy);

    ~AlpineStackConfig () = default;

    AlpineStackConfig & operator = (const AlpineStackConfig & copy);



    // Various configuration options
    //
    bool  setLocalEndpoint (const string &  ipAddress,
                            ushort          port);      // for now, only a single endpoint supported.

    bool  getLocalEndpoint (string &   ipAddress,
                            ushort &  port);

    bool  setLocalEndpoint (ulong   ipAddress,
                            ushort  port);

    bool  getLocalEndpoint (ulong &   ipAddress,
                            ushort &  port);

    bool  setMaxConcurrentQueries (ulong  max);

    bool  getMaxConcurrentQueries (ulong &  max);



  private:

    ulong   localIpAddress_;
    ushort  localPort_;
    ulong   maxConcurrentQueries_;

};

