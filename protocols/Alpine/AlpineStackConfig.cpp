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


#include <AlpineStackConfig.h>
#include <NetUtils.h>
#include <Log.h>
#include <StringUtils.h>



AlpineStackConfig::AlpineStackConfig ()
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStackConfig constructor invoked.");
#endif

    localIpAddress_        = 0;
    localPort_             = 0;
    maxConcurrentQueries_  = 0;
}



AlpineStackConfig::AlpineStackConfig (const AlpineStackConfig & copy)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStackConfig copy constructor invoked.");
#endif

    localIpAddress_        = copy.localIpAddress_;
    localPort_             = copy.localPort_;
    maxConcurrentQueries_  = copy.maxConcurrentQueries_;
}



// Dtor defaulted in header



AlpineStackConfig & 
AlpineStackConfig::operator = (const AlpineStackConfig & copy)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStackConfig assignment invoked.");
#endif

    if (&copy == this) {
        return *this;
    }

    localIpAddress_        = copy.localIpAddress_;
    localPort_             = copy.localPort_;
    maxConcurrentQueries_  = copy.maxConcurrentQueries_;

    return *this;
}



bool  
AlpineStackConfig::setLocalEndpoint (const string &  ipAddress,
                                     ushort          port)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStackConfig::setLocalEndpoint invoked.  Values: "s +
                "\n IP Address: "s + ipAddress +
                "\n Port: "s + std::to_string (port) +
                "\n");
#endif

    bool   status;
    ulong  localIp;

    status = NetUtils::stringIpToLong (ipAddress, localIp);

    if (!status) {
        Log::Error ("Invalid IP address passed in call to AlpineStackConfig::setLocalEndpoint!");
        return false;
    }

    if (port == 0) {
        Log::Error ("Invalid port passed in call to AlpineStackConfig::setLocalEndpoint!");
        return false;
    }

    localIpAddress_ = localIp;
    localPort_      = port;
    

    return true;
}



bool  
AlpineStackConfig::getLocalEndpoint (string &   ipAddress,
                                     ushort &  port)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStackConfig::getLocalEndpoint invoked.");
#endif

    if (localIpAddress_ == 0) {
        Log::Error ("Call to AlpineStackConfig::getLocalEndpoint before initialization!");
        return false;
    }

    NetUtils::longIpToString (localIpAddress_, ipAddress);
    port = localPort_;


    return true;
}



bool  
AlpineStackConfig::setLocalEndpoint (ulong   ipAddress,
                                     ushort  port)
{
#ifdef _VERBOSE
    string ipAddressString;
    NetUtils::longIpToString (ipAddress, ipAddressString);

    Log::Debug ("AlpineStackConfig::setLocalEndpoint invoked.  Values: "s +
                "\n IP Address: "s + ipAddressString +
                "\n Port: "s + std::to_string (port) +
                "\n");
#endif

    if (ipAddress == 0) {
        Log::Error ("Invalid IP address passed in call to AlpineStackConfig::setLocalEndpoint!");
        return false;
    }

    if (port == 0) {
        Log::Error ("Invalid port passed in call to AlpineStackConfig::setLocalEndpoint!");
        return false;
    }

    localIpAddress_ = ipAddress;
    localPort_      = port;


    return true;
}



bool  
AlpineStackConfig::getLocalEndpoint (ulong &   ipAddress,
                                     ushort &  port)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStackConfig::getLocalEndpoint invoked.");
#endif

    if (localIpAddress_ == 0) {
        Log::Error ("Call to AlpineStackConfig::getLocalEndpoint before initialization!");
        return false;
    }

    ipAddress = localIpAddress_;
    port      = localPort_;


    return true;
}



bool  
AlpineStackConfig::setMaxConcurrentQueries (ulong  max)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStackConfig::setMaxConcurrentQueries invoked.  Max: "s +
                std::to_string (max));
#endif

    if (max == 0) {
        Log::Error ("Invalid maximum passed in call to AlpineStackConfig::setMaxConcurrentQueries!");
        return false;
    }

    maxConcurrentQueries_ = max;
    

    return true;
}



bool  
AlpineStackConfig::getMaxConcurrentQueries (ulong &  max)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStackConfig::getMaxConcurrentQueries invoked.");
#endif

    if (maxConcurrentQueries_ == 0) {
        Log::Error ("Call to AlpineStackConfig::getMaxConcurrentQueries before initialization!");
        return false;
    }

    max = maxConcurrentQueries_;


    return true;
}



