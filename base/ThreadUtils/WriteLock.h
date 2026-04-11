/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <ReadWriteSem.h>
#include <mutex>
#include <shared_mutex>


class WriteLock
{
  public:
    explicit WriteLock(ReadWriteSem & semaphore)
        : lock_(semaphore.native())
    {}


  private:
    std::unique_lock<std::shared_mutex> lock_;
};
