/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <AutoThread.h>


class TestThread : public AutoThread
{
  public:

    TestThread (const string  message);

    virtual ~TestThread ();


    virtual void threadMain ();


  private:

    string msg_;

};

