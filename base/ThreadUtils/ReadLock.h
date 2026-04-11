/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <ReadWriteSem.h>
#include <shared_mutex>


class ReadLock
{
  public:
    explicit ReadLock(ReadWriteSem & semaphore)
        : lock_(semaphore.native())
    {}


  private:
    std::shared_lock<std::shared_mutex> lock_;
};
