/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <SysThread.h>


class AlpineServiceThread : public SysThread
{
  public:
    AlpineServiceThread() = default;
    virtual ~AlpineServiceThread();


    virtual void threadMain();


  private:
};
