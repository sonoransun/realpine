/// Copyright (C) 2026 sonoransun — see LICENCE.txt


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


