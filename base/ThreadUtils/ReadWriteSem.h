/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <shared_mutex>


class ReadWriteSem
{
  public:
    void
    acquireRead()
    {
        mutex_.lock_shared();
    }

    void
    acquireWrite()
    {
        mutex_.lock();
    }

    bool
    tryAcquireRead()
    {
        return mutex_.try_lock_shared();
    }

    bool
    tryAcquireWrite()
    {
        return mutex_.try_lock();
    }

    void
    releaseRead()
    {
        mutex_.unlock_shared();
    }

    void
    releaseWrite()
    {
        mutex_.unlock();
    }

    std::shared_mutex &
    native()
    {
        return mutex_;
    }


  private:
    std::shared_mutex mutex_;
};
