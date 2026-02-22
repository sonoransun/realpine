/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <SysThread.h>


class TcpTransport;


class TcpClientThread : public SysThread
{
  public:

    TcpClientThread (TcpTransport *  transport);

    virtual ~TcpClientThread ();


    virtual void threadMain ();


  private:

    TcpTransport *  transport_;

};

