/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <SysThread.h>
#include <ApplCore.h>


class SignalMonitorThread : public SysThread
{
  public:

    SignalMonitorThread () = default;
    virtual ~SignalMonitorThread ();

  
    virtual void threadMain ();


  private:

};

