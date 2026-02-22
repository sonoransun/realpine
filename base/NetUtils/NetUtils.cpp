/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <NetUtils.h>
#include <Log.h>
#include <StringUtils.h>
#include <ctype.h>
#include <cstdio>



bool 
NetUtils::stringIpToLong (const string &  strIpAddress,
                          ulong &         longIpAddress)
{
    return (inet_addr_r (strIpAddress.c_str(), longIpAddress));
}



bool 
NetUtils::longIpToString (const ulong  longIpAddress,
                          string &     strIpAddress)
{
    const int buffLen = 64;
    char tmpBuff[buffLen];

    ulong netIpAddr = ntohl (longIpAddress);
    bool retVal = inet_ntoa_r (netIpAddr, tmpBuff, buffLen);

    if (retVal) {
        strIpAddress = tmpBuff;
    }

    return retVal;    
}



bool 
NetUtils::inet_addr_r (const char *  strIpAddress,
                       ulong &       netIpAddress)
{
    const char *currChar;
    ulong val;
    unsigned char n;
    unsigned int  tempVal;
    char c;


    netIpAddress = 0;

    // verify format
    currChar = strIpAddress;
    int decimalCount = 0;

    if (!isdigit(*currChar)) {
        // first char must be a digit
        return false;
    }
    tempVal = 0;

    while (*currChar) {
        if (*currChar == '.') {
            if (*(currChar + 1) == '.') {
                // missing a number between consecutive '.' separators.
                return false;
            }
            tempVal = 0;
            decimalCount++;
        }
        else if (!isdigit(*currChar)) {
            // bad format...
            return false;
        }
        else {
            tempVal = (tempVal * 10) + (*currChar - '0');

            if (tempVal > 255) {
                // maximum unsigned value exceeded, invalid number.
                return false;
            }
        }

        currChar++;
    }

    if (decimalCount != 3) {
        return false;
    }
    // convert values...
    currChar = strIpAddress;

    bool done = false;
    val = 0;
    n = 0;

    while (!done) {

        c = *currChar;

        if (isascii(c) && isdigit(c)) {
            n = (n * 10) + (c - '0');  // shift and add
            currChar++;
            continue;
        }
        else if (c == '.') {
            if (val) {
                val <<= 8;
            }

            val += n;
            n = 0;
            currChar++;
            continue;
        }
        else {
            done = true;
        }
    }

    val <<= 8;
    val += n;


    netIpAddress = htonl(val);


    return true;
}



bool 
NetUtils::inet_ntoa_r (const ulong  netIpAddress,
                       char *       outStrIpAddress,
                       int          outBuffLen)
{
    char *byteArr;
    int expLength;
    ulong tempAddr;

    tempAddr = ntohl(netIpAddress);
    byteArr = (char *)&tempAddr;

#define AS_BYTE(b)   (((int)b)&0xff)

    expLength = snprintf(outStrIpAddress, outBuffLen, "%d.%d.%d.%d",
                         AS_BYTE(byteArr[0]),
                         AS_BYTE(byteArr[1]),
                         AS_BYTE(byteArr[2]),
                         AS_BYTE(byteArr[3]));

#undef AS_BYTE

    if (expLength > outBuffLen) {
        // not enough room...
        return false;
    }
    return true;
}



void 
NetUtils::socketErrorAsString (const int  errorCode,
                               string &   errorString)
{
    switch (errorCode) {
      case EACCES :
        errorString = "No priveleges for this operation";
        break;

      case EAFNOSUPPORT :
        errorString = "Address family not supported";
        break;

      case EMFILE :
        errorString = "No more file descriptors available";
        break;

      case EPROTONOSUPPORT :
        errorString = "Protocol not supported";
        break;

      case EPROTOTYPE :
        errorString = "Socket type not supported by protocol";
        break;

      case ENOBUFS :
        errorString = "Insufficient system resources";
        break;

      case ENOMEM :
        errorString = "Insufficient system memory";
        break;

      case EBADF :
        errorString = "Bad file descriptor";
        break;

      case ECONNRESET :
        errorString = "Connection reset by peer";
        break;

      case EINTR :
        errorString = "Signal interrupted operation";
        break;

      case EMSGSIZE :
        errorString = "Data too large for operation";
        break;

      case ENOTSOCK :
        errorString = "Argument is not a socket";
        break;

      case EOPNOTSUPP :
        errorString = "Operation is not supported for this socket";
        break;

      case EPIPE :
        errorString = "Broken pipe detected on the socket";
        break;

      case EAGAIN :
        errorString = "NONBLOCK mode is set and no data is waiting";
        break;

      case EIO :
        errorString = "Filesystem I/O error";
        break;

      case ELOOP :
        errorString = "Too many symbolic links resolving named socket";
        break;

      case ENAMETOOLONG :
        errorString = "Full path name for socket exceedes PATH_MAX";
        break;

      case ENOENT :
        errorString = "Invalid named socket filename";
        break;

      case ENOTDIR :
        errorString = "Path prefix component is not a directory";
        break;

      case EDESTADDRREQ :
        errorString = "Invalid destination address";
        break;

      case EHOSTUNREACH :
        errorString = "Host is unreachable";
        break;

      case EINVAL :
        errorString = "Invalid destination address length";
        break;

      case EISCONN :
        errorString = "Socket connected and destination address specified";
        break;

      case ENETDOWN :
        errorString = "Local network interface is down";
        break;

      case ENETUNREACH :
        errorString = "No route to destination";
        break;

      case ECONNREFUSED :
        errorString = "Connection refused";
        break;

      case EHOSTDOWN :
        errorString = "Host is down";
        break;



      default:
        errorString = "Unknown error code: " + std::to_string(errorCode);
        break;
    }
}



bool
NetUtils::nonBlocking (int  fd)
{
#ifdef _VERBOSE
    Log::Debug ("NetUtils::nonBlocking invoked.");
#endif

    if (!alpine_set_nonblocking(fd)) {
        Log::Debug ("Error setting non blocking for FD in call to "
                    "NetUtils::nonBlocking.");
        return false;
    }
    return true;
}



bool
NetUtils::blocking (int  fd)
{
#ifdef _VERBOSE
    Log::Debug ("NetUtils::blocking invoked.");
#endif

    if (!alpine_set_blocking(fd)) {
        Log::Debug ("Error clearing file flags for FD in call to "
                    "NetUtils::blocking.");
        return false;
    }
    return true;
}



bool
NetUtils::getLocalEndpoint (int       socketFd,
                            ulong &   ipAddress,
                            ushort &  port)
{
#ifdef _VERBOSE
    Log::Debug ("NetUtils::getLocalEndpoint invoked.");
#endif

    struct sockaddr_in  localAddr;
    socklen_t localAddrSize = sizeof(localAddr);

    int retVal;
    retVal = getsockname (socketFd,
                          reinterpret_cast<struct sockaddr *>(&localAddr),
                          &localAddrSize);

    if (retVal < 0) {
        string errorCode;
        NetUtils::socketErrorAsString (alpine_socket_errno(), errorCode);
        Log::Error ("getsockname error: "s + errorCode +
                    " in call to NetUtils::getLocalEndpoint!");

        return false;
    }
    if (localAddr.sin_family != AF_INET) {
        Log::Error ("Invalid address family returned in call to "
                             "NetUtils::getLocalEndpoint!");
        return false;
    }
    ipAddress  = localAddr.sin_addr.s_addr;
    port       = localAddr.sin_port;


    return true;
}



bool
NetUtils::getFarEndpoint (int       socketFd,
                          ulong &   ipAddress,
                          ushort &  port)
{
#ifdef _VERBOSE
    Log::Debug ("NetUtils::getFarEndpoint invoked.");
#endif

    struct sockaddr_in  farAddr;
    socklen_t farAddrSize = sizeof(farAddr);

    int retVal;
    retVal = getpeername (socketFd,
                          reinterpret_cast<struct sockaddr *>(&farAddr),
                          &farAddrSize);

    if (retVal < 0) {
        string errorCode;
        NetUtils::socketErrorAsString (alpine_socket_errno(), errorCode);
        Log::Error ("getsockname error: "s + errorCode +
                    " in call to NetUtils::getFarEndpoint!");

        return false;
    }
    if (farAddr.sin_family != AF_INET) {
        Log::Error ("Invalid address family returned in call to "
                             "NetUtils::getFarEndpoint!");
        return false;
    }
    ipAddress  = farAddr.sin_addr.s_addr;
    port       = farAddr.sin_port;


    return true;
}



