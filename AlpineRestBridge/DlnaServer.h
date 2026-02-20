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
#include <TcpAcceptor.h>
#include <TcpTransport.h>
#include <ContentStore.h>
#include <memory>


class DlnaServer : public SysThread
{
  public:

    DlnaServer (ContentStore & store);
    ~DlnaServer ();

    bool    initialize (ushort port, const string & hostAddress,
                        bool transcodeEnabled);

    void    threadMain ();

    ushort  getPort ();
    string  getBaseUrl ();
    string  getDeviceUuid ();


  private:

    void  handleConnection (std::unique_ptr<TcpTransport> transport);

    void  sendResponse (TcpTransport * t, int status,
                        const string & contentType, const string & body);

    static string  generateUuid (const string & hostname, ushort port);

    ContentStore &  store_;
    TcpAcceptor     acceptor_;
    ushort          port_;
    string          hostAddress_;
    string          baseUrl_;
    string          deviceUuid_;
    bool            transcodeEnabled_;

};
