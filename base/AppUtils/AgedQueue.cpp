/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AgedQueue.h>
#include <Log.h>
#include <StringUtils.h>


static const int  linkEnd = -1;


AgedQueue::AgedQueue (t_Size reserve)
{
    (reserve <= 0) ? size_ = 1024 : size_ = reserve;
    numRecords_   = 0;
    newestIndex_  = linkEnd;
    oldestIndex_  = linkEnd;


    // Initialize containers...
    //
    recordList_.resize (size_);

    t_Size  i;
    for (i = 0; i < size_; i++) {
        freeIndexList_.push_back (i);
        recordList_[i].reference = nullptr;
        recordList_[i].next = linkEnd;
        recordList_[i].prev = linkEnd;
    }
}



bool
AgedQueue::clear ()
{
    numRecords_   = 0;
    newestIndex_  = linkEnd;
    oldestIndex_  = linkEnd;

    recordList_.clear ();
    recordList_.resize (size_);

    freeIndexList_.clear ();

    ageIndex_.clear ();

    t_Size  i;
    for (i = 0; i < size_; i++) {
        freeIndexList_.push_back (i);
        recordList_[i].reference = nullptr;
        recordList_[i].next = linkEnd;
        recordList_[i].prev = linkEnd;
    }


    return true;
}



AgedQueue::t_Size
AgedQueue::size ()
{
    return numRecords_;
}



bool
AgedQueue::add (void * reference)
{
#ifdef _VERBOSE
    Log::Debug ("AgedQueue::add invoked.");
#endif

    if (exists (reference)) {
        return false;
    }

    // Resize if needed...
    //
    if (numRecords_ >= size_) {
        resize (size_ * 2);
    }

    t_Size         currIndex   =  freeIndexList_.front ();
    freeIndexList_.pop_front ();
    t_AgeRecord *  currRecord  = &recordList_[currIndex];

    currRecord->reference = reference;

    ageIndex_.insert ( t_AgeIndexPair(reference, currIndex) );

    if (numRecords_ == 0) {

        // First record...
        //
        newestIndex_ = oldestIndex_ = currIndex;

        currRecord->next = linkEnd;
        currRecord->prev = linkEnd;

        numRecords_++;

        return true;
    }

    // Not the first one...
    //
    t_AgeRecord *  newestRecord = &recordList_[newestIndex_];

    newestRecord->next = currIndex;
    currRecord->next = linkEnd;
    currRecord->prev = newestIndex_;

    newestIndex_ = currIndex;
    numRecords_++;


    return true;
}



bool
AgedQueue::touch (void * reference)
{
    if (!exists (reference)) {
        return false;
    }

    // touch == Move this record to the front of the age queue.
    //
    auto indexIter = ageIndex_.find (reference);
    t_Size currIndex = (*indexIter).second;


    // If this record is currently the newest, no need to reorder queue.
    //
    if ((*indexIter).second == newestIndex_) {
        return true;
    }

    t_AgeRecord *  currRecord   = &recordList_[currIndex];
    t_AgeRecord *  prevRecord;
    t_AgeRecord *  nextRecord;


    // If this is the oldest record, update oldest index.
    //
    if (oldestIndex_ == currIndex) {
        oldestIndex_ = currRecord->next;
    }


    // Pull current record from doubly linked list...
    //
    if (currRecord->prev != linkEnd) {
        prevRecord = &recordList_[currRecord->prev];
        prevRecord->next = currRecord->next;
    }
    if (currRecord->next != linkEnd) {
        nextRecord = &recordList_[currRecord->next];
        nextRecord->prev = currRecord->prev;
    }


    // Push updated record to front of age queue...
    //
    t_AgeRecord *  newestRecord = &recordList_[newestIndex_];

    newestRecord->next = currIndex;
    currRecord->next = linkEnd;
    currRecord->prev = newestIndex_;

    newestIndex_ = currIndex;


    return true;
}



bool
AgedQueue::remove (void * reference)
{
    if (numRecords_ == 0) {
        return false;
    }

    auto indexIter = ageIndex_.find (reference);
    t_Size currIndex = (*indexIter).second;

    ageIndex_.erase (reference);


    t_AgeRecord *  currRecord   = &recordList_[currIndex];
    t_AgeRecord *  prevRecord;
    t_AgeRecord *  nextRecord;


    // If this is the oldest or newest record, update respective index.
    //
    if (oldestIndex_ == currIndex) {
        oldestIndex_ = currRecord->next;
    }
    if (newestIndex_ == currIndex) {
        newestIndex_ = currRecord->prev;
    }


    // Pull current record from doubly linked list...
    //
    if (currRecord->prev != linkEnd) {
        prevRecord = &recordList_[currRecord->prev];
        prevRecord->next = currRecord->next;
    }
    if (currRecord->next != linkEnd) {
        nextRecord = &recordList_[currRecord->next];
        nextRecord->prev = currRecord->prev;
    }

    freeIndexList_.push_back (currIndex);
    numRecords_--;


    return true;
}



bool
AgedQueue::exists (void * reference)
{
    if (numRecords_ == 0) {
        return false;
    }

    return ageIndex_.find (reference) != ageIndex_.end();
}



bool
AgedQueue::newest (void *& reference)
{
    if (numRecords_ == 0) {
        return false;
    }

    t_AgeRecord *  currRecord = &recordList_[newestIndex_];
    reference = currRecord->reference;

    return true;
}



bool
AgedQueue::oldest (void *& reference)
{
    if (numRecords_ == 0) {
        return false;
    }

    t_AgeRecord *  currRecord = &recordList_[oldestIndex_];
    reference = currRecord->reference;

    return true;
}



bool
AgedQueue::next (void *& reference)
{
    if (!exists (reference)) {
        return false;
    }

    auto indexIter = ageIndex_.find (reference);
    t_Size currIndex = (*indexIter).second;

    t_AgeRecord *  currRecord = &recordList_[currIndex];

    if (currRecord->next == linkEnd) {
        // End of list...
        return false;
    }

    t_AgeRecord *  nextRecord = &recordList_[currRecord->next];
    reference = nextRecord->reference;


    return true;
}



bool
AgedQueue::prev (void *& reference)
{
    if (!exists (reference)) {
        return false;
    }

    auto indexIter = ageIndex_.find (reference);
    t_Size currIndex = (*indexIter).second;

    t_AgeRecord *  currRecord = &recordList_[currIndex];

    if (currRecord->prev == linkEnd) {
        // End of list...
        return false;
    }

    t_AgeRecord *  prevRecord = &recordList_[currRecord->prev];
    reference = prevRecord->reference;


    return true;
}



bool
AgedQueue::resize (t_Size size)
{
    // Resize containers...
    // This method should only be called when the current containers are FULL
    //

    t_AgeRecordList  newRecordList;
    newRecordList.resize (size);

    t_FreeIndexList  newFreeIndexList;

    // Copy existing data...
    //
    t_Size i;
    for (i = 0; i < size_; i++) {
        newRecordList[i].reference = recordList_[i].reference;
        newRecordList[i].prev = recordList_[i].prev;
        newRecordList[i].next = recordList_[i].next;
    }

    // Initialize remaining records...
    for (i = size_; i < size; i++) {
        newFreeIndexList.push_back (i);
        newRecordList[i].reference = nullptr;
        newRecordList[i].prev = linkEnd;
        newRecordList[i].next = linkEnd;
    }

    size_ = size;

    recordList_ = std::move (newRecordList);
    freeIndexList_ = std::move (newFreeIndexList);


    return true;
}



