/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <SysThread.h>


class TestThread : public SysThread
{
  public:

    TestThread (const string  message);

    virtual ~TestThread ();


    virtual void threadMain ();


  private:

    string msg_;

};

