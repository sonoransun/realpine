/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <SignalSet.h>


class ThreadSigMask
{
  public:
    ThreadSigMask() = default;
    ~ThreadSigMask() = default;


    static bool setSigMask(SignalSet & sigSet);


  private:
};
