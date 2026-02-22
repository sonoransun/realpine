/// Copyright (C) 2026 sonoransun — see LICENCE.txt
    

#include <PollSet.h>


static const int defaultPollFds = 128;


PollSet::PollSet (short  requestedEvents)
{
    requestedEvents_ = requestedEvents;

    maxPollFds_ = defaultPollFds;
    numPollFds_ = 0;

    pollFdList_ = new struct pollfd [maxPollFds_];
}



PollSet::~PollSet ()
{
    delete [] pollFdList_;
}



void
PollSet::clear ()
{
    numPollFds_ = 0;
}



bool
PollSet::add (int fileDesc)
{
    if ( (numPollFds_ + 1) >= maxPollFds_ ) {
       resizePollList (maxPollFds_ * 2);
    }

    pollFdList_[numPollFds_].fd     = fileDesc;
    pollFdList_[numPollFds_].events = requestedEvents_;

    numPollFds_++;

    return true;
}



bool
PollSet::add (const t_FileDescList &  fileDescList)
{
    if ( (numPollFds_ + (int)fileDescList.size()) >= maxPollFds_ ) {
        resizePollList (maxPollFds_ + fileDescList.size());
    }

    int i;

    for (i = 0; i < (int)fileDescList.size(); i++) {
        pollFdList_[numPollFds_].fd     = fileDescList[i];
        pollFdList_[numPollFds_].events = requestedEvents_;
        numPollFds_++;
    }

    return true;
}



bool 
PollSet::getFdList (t_FileDescList &  fileDescList)
{
    fileDescList.resize (numPollFds_);

    int i;
    for (i = 0; i < numPollFds_; i++) {
        fileDescList[i]  = pollFdList_[i].fd;
    }
    
    return true;
}



int
PollSet::size ()
{
    return numPollFds_;
}



bool 
PollSet::setEvents (short  requestedEvents)
{
    requestedEvents_ =  requestedEvents;

    int i;
    for (i = 0; i < numPollFds_; i++) {
        pollFdList_[i].events = requestedEvents_;
    }

    return true;
}



bool
PollSet::poll (int               timeout,
               t_FileDescList &  activeFileDescList)
{
    int numActive;

    numActive = alpine_poll(pollFdList_,
                            numPollFds_,
                            timeout);

    if (numActive < 0) {
        // Poll failed?
        return false;
    }
    if (numActive == 0) {
        // nothing pending...
        return true;
    }


    // return list of all active FD's
    //
    int i;
    activeFileDescList.clear ();

    for (i = 0; i < numPollFds_; i++) {
        if (pollFdList_[i].revents) {
            activeFileDescList.push_back (pollFdList_[i].fd);
        }
    }


    return true;
}



bool
PollSet::resizePollList (int extent)
{
    maxPollFds_ = extent;

    struct pollfd *  newPollFdList;
    newPollFdList = new struct pollfd [maxPollFds_];

    // Copy data
    int i;
    for (i = 0; i < numPollFds_; i++) {
         newPollFdList[i].fd     = pollFdList_[i].fd;
         newPollFdList[i].events = requestedEvents_;
    }

    delete [] pollFdList_;
    pollFdList_ = newPollFdList;

    return true;
}



