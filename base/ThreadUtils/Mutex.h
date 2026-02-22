/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <mutex>


class Mutex
{
  public:

    void  acquire ()      { mutex_.lock(); }

    bool  tryAcquire ()   { return mutex_.try_lock(); }

    void  release ()      { mutex_.unlock(); }

    std::mutex &  native ()  { return mutex_; }


  private:

    std::mutex  mutex_;

};
