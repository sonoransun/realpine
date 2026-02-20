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
#include <SysThread.h>


class WifiBeaconInjector : public SysThread
{
  public:

    WifiBeaconInjector ();
    ~WifiBeaconInjector ();

    bool  initialize (const string & interfaceName, const string & peerId,
                      const string & ipAddress, ushort port, ushort restPort,
                      uint capabilities, int beaconIntervalMs);

    void  threadMain ();


  private:

#ifdef __linux__
    bool  openRawSocket ();
    uint  buildBeaconFrame (byte * buffer, uint bufferSize);
    uint  buildVendorIE (byte * buffer, uint offset);
    void  parseBeaconFrame (const byte * data, uint length);
    bool  parseVendorIE (const byte * ie, uint ieLen);

    static constexpr byte ALPINE_OUI[3] = { 0x02, 0xA1, 0xE1 };
    static constexpr byte ALPINE_OUI_TYPE = 0x01;

    int     rawFd_;
    int     ifIndex_;
    byte    localMac_[6];
#endif

    static const int  POLL_TIMEOUT_MS = 500;

    string  interfaceName_;
    string  peerId_;
    string  ipAddress_;
    ushort  port_;
    ushort  restPort_;
    uint    capabilities_;
    int     beaconIntervalMs_;

};
