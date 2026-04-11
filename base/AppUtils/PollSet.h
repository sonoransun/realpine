/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <Platform.h>
#include <vector>


class PollSet
{
  public:
    PollSet(short requestedEvents = POLLIN);
    ~PollSet();


    ////
    //
    // PollSet types and operations...
    //
    using t_FileDescList = vector<int>;


    void clear();

    bool add(int fileDesc);

    bool add(const t_FileDescList & fileDescList);

    bool getFdList(t_FileDescList & fileDescList);

    int size();

    bool setEvents(short requestedEvents);

    bool poll(int timeout, t_FileDescList & activeFileDescList);


  private:
    int requestedEvents_;
    int maxPollFds_;
    int numPollFds_;
    struct pollfd * pollFdList_;


    bool resizePollList(int extent);
};
