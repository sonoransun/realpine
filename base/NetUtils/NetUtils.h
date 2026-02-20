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
#include <Platform.h>


class NetUtils {

  public:

    static bool stringIpToLong (const string &  strIpAddress,
                                ulong &         longIpAddress);

    static bool longIpToString (const ulong  longIpAddress,
                                string &     strIpAddress);

    static void socketErrorAsString (const int  errorCode,
                                     string &   errorString);    

    static bool nonBlocking (int fd);

    static bool blocking (int fd);

    static bool getLocalEndpoint (int       socketFd,
                                  ulong &   ipAddress,
                                  ushort &  port);

    static bool getFarEndpoint (int       socketFd,
                                ulong &   ipAddress,
                                ushort &  port);



  private:

    static bool inet_addr_r (const char *  strIpAddress,
                             ulong &       netIpAddress);    

    static bool inet_aton_r (const char *  strIpAddress,
                             ulong &       netIpAddress)
    {
        return ( NetUtils::inet_addr_r (strIpAddress, netIpAddress) );
    }

    static bool inet_ntoa_r (const ulong  netIpAddress,
                             char *       outStrIpAddress,
                             int          outBuffLen);

};


