///////
///
///  Copyright (C) 2026  sonoransun
///
///  Permission is hereby granted, free of charge, to any person obtaining a copy
///  of this software and associated documentation files (the "Software"), to deal
///  in the Software without restriction, including without limitation the rights
///  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
///  copies of the Software, and to permit persons to whom the Software is
///  furnished to do so, subject to the following conditions:
///
///  The above copyright notice and this permission notice shall be included in all
///  copies or substantial portions of the Software.
///
///  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
///  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
///  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
///  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
///  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
///  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
///  SOFTWARE.
///
///////


#pragma once
#include <Common.h>
#include <Platform.h>
#include <vector>


class PollSet
{
  public:

    PollSet (short requestedEvents = POLLIN);
    ~PollSet ();


    ////
    // 
    // PollSet types and operations...
    //
    using t_FileDescList = vector<int>;


    void clear ();

    bool add (int fileDesc);

    bool add (const t_FileDescList &  fileDescList);

    bool getFdList (t_FileDescList &  fileDescList);

    int  size ();

    bool setEvents (short  requestedEvents);

    bool poll (int               timeout,
               t_FileDescList &  activeFileDescList);


  private:

    int requestedEvents_;
    int maxPollFds_;
    int numPollFds_;
    struct pollfd * pollFdList_;
   
    
    bool resizePollList (int extent); 

};


