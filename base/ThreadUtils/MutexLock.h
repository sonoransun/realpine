/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Mutex.h>
#include <mutex>


class MutexLock
{
  public:
    explicit MutexLock(Mutex & mutex)
        : lock_(mutex.native())
    {}


  private:
    std::lock_guard<std::mutex> lock_;
};
