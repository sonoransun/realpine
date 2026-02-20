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
#include <OptHash.h>
#include <vector>
#include <list>


class AgedQueue
{
  public:

    using t_Size = int;

    AgedQueue (t_Size reserve = 1024);
    ~AgedQueue ();


    bool  clear ();

    t_Size  size ();

    bool  add (void * reference);

    bool  touch (void * reference);

    bool  remove (void * reference);

    bool  exists (void * reference);

    bool  newest (void *& reference);

    bool  oldest (void *& reference);

    // For these operations, reference must point to an existing record.
    //
    bool  next (void *& reference);

    bool  prev (void *& reference);


    ////
    //
    // Types
    //
    struct t_AgeRecord {
        void *  reference;
        t_Size  next;
        t_Size  prev;
    };

    using t_AgeRecordList = vector<t_AgeRecord>;

    using t_FreeIndexList = list<t_Size>;

    using t_AgeIndex = std::unordered_map< void *,
                      t_Size,
                      OptHash<void *>,
                      equal_to<void *> >;

    using t_AgeIndexPair = std::pair<void *, t_Size>;


  private:

    t_Size             size_;
    t_Size             numRecords_;
    t_AgeRecordList *  recordList_;
    t_FreeIndexList *  freeIndexList_;
    t_AgeIndex *       ageIndex_;
    t_Size             newestIndex_;
    t_Size             oldestIndex_;

    bool  resize (t_Size size);

};


