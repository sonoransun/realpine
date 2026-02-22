/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <SysThread.h>


class OrbEventThread : public SysThread
{
  public:

    OrbEventThread () = default;
    virtual ~OrbEventThread ();


    virtual void threadMain ();

  private:

};

