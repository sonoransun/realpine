/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <TcpAsyncConnector.h>


class TcpClientConnector : public TcpAsyncConnector
{
  public:

    TcpClientConnector () {};
    virtual ~TcpClientConnector () {};


    virtual void  receiveTransport (ulong           requestId,
                                    TcpTransport *  transport);

    virtual void  handleRequestFailure (ulong  requestId);


  private:

};

