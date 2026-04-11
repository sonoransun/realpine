/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <OptHash.h>
#include <list>
#include <vector>


class AgedQueue
{
  public:
    using t_Size = int;

    AgedQueue(t_Size reserve = 1024);
    ~AgedQueue() = default;

    AgedQueue(const AgedQueue &) = delete;
    AgedQueue & operator=(const AgedQueue &) = delete;


    bool clear();

    t_Size size();

    bool add(void * reference);

    bool touch(void * reference);

    bool remove(void * reference);

    bool exists(void * reference);

    bool newest(void *& reference);

    bool oldest(void *& reference);

    // For these operations, reference must point to an existing record.
    //
    bool next(void *& reference);

    bool prev(void *& reference);


    ////
    //
    // Types
    //
    struct t_AgeRecord
    {
        void * reference;
        t_Size next;
        t_Size prev;
    };

    using t_AgeRecordList = vector<t_AgeRecord>;

    using t_FreeIndexList = list<t_Size>;

    using t_AgeIndex = std::unordered_map<void *, t_Size, OptHash<void *>, equal_to<void *>>;

    using t_AgeIndexPair = std::pair<void *, t_Size>;


  private:
    t_Size size_;
    t_Size numRecords_;
    t_AgeRecordList recordList_;
    t_FreeIndexList freeIndexList_;
    t_AgeIndex ageIndex_;
    t_Size newestIndex_;
    t_Size oldestIndex_;

    bool resize(t_Size size);
};
