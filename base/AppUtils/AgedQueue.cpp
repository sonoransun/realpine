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


    // Allocate containers...
    //
    recordList_     = nullptr;
    freeIndexList_  = nullptr;
    ageIndex_       = nullptr;

    recordList_ = new t_AgeRecordList;
    recordList_->clear ();
    recordList_->resize (size_);

    freeIndexList_ = new t_FreeIndexList;
    freeIndexList_->clear ();

    ageIndex_ = new t_AgeIndex;
    ageIndex_->clear ();


    // Initialize containers...
    //
    t_Size  i;
    for (i = 0; i < size_; i++) {
        freeIndexList_->push_back (i);
        (*recordList_)[i].reference = nullptr;
        (*recordList_)[i].next = linkEnd;
        (*recordList_)[i].prev = linkEnd;
    }
}



AgedQueue::~AgedQueue ()
{
    if (recordList_) {
        delete recordList_;
    }
    if (freeIndexList_) {
        delete freeIndexList_;
    }
    if (ageIndex_) {
        delete ageIndex_;
    }
}



bool  
AgedQueue::clear ()
{
    delete recordList_;
    delete freeIndexList_;
    delete ageIndex_;

    numRecords_   = 0;
    newestIndex_  = linkEnd;
    oldestIndex_  = linkEnd;

    recordList_ = new t_AgeRecordList;
    recordList_->clear ();
    recordList_->reserve (size_);

    freeIndexList_ = new t_FreeIndexList;
    freeIndexList_->clear ();

    ageIndex_ = new t_AgeIndex;
    ageIndex_->clear ();

    t_Size  i;
    for (i = 0; i < size_; i++) {
        freeIndexList_->push_back (i);
        (*recordList_)[i].reference = nullptr;
        (*recordList_)[i].next = linkEnd;
        (*recordList_)[i].prev = linkEnd;
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

    t_Size         currIndex   =  freeIndexList_->front ();
    freeIndexList_->pop_front ();
    t_AgeRecord *  currRecord  = (t_AgeRecord *) &( (*recordList_)[currIndex] );

    currRecord->reference = reference;

    ageIndex_->insert ( t_AgeIndexPair(reference, currIndex) );

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
    t_AgeRecord *  newestRecord = (t_AgeRecord *) &( (*recordList_)[newestIndex_] );

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
    auto indexIter = ageIndex_->find (reference);
    t_Size currIndex = (*indexIter).second;


    // If this record is currently the newest, no need to reorder queue.
    //
    if ((*indexIter).second == newestIndex_) {
        return true; 
    }

    t_AgeRecord *  currRecord   = (t_AgeRecord *) &( (*recordList_)[currIndex] );
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
        prevRecord = &( (*recordList_)[currRecord->prev] );
        prevRecord->next = currRecord->next;
    }
    if (currRecord->next != linkEnd) {
        nextRecord = &( (*recordList_)[currRecord->next] );
        nextRecord->prev = currRecord->prev;
    }


    // Push updated record to front of age queue...
    //
    t_AgeRecord *  newestRecord = (t_AgeRecord *) &( (*recordList_)[newestIndex_] );

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

    auto indexIter = ageIndex_->find (reference);
    t_Size currIndex = (*indexIter).second;

    ageIndex_->erase (reference);


    t_AgeRecord *  currRecord   = (t_AgeRecord *) &( (*recordList_)[currIndex] );
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
        prevRecord = (t_AgeRecord *) &( (*recordList_)[currRecord->prev] );
        prevRecord->next = currRecord->next;
    }
    if (currRecord->next != linkEnd) {
        nextRecord = (t_AgeRecord *) &( (*recordList_)[currRecord->next] );
        nextRecord->prev = currRecord->prev;
    }

    freeIndexList_->push_back (currIndex);
    numRecords_--;


    return true;
}



bool  
AgedQueue::exists (void * reference)
{
    if (numRecords_ == 0) {
        return false;
    }

    return ageIndex_->find (reference) != ageIndex_->end();
}



bool  
AgedQueue::newest (void *& reference)
{
    if (numRecords_ == 0) {
        return false;
    }

    t_AgeRecord *  currRecord = (t_AgeRecord *) &( (*recordList_)[newestIndex_] );
    reference = currRecord->reference;
    
    return true;
}



bool  
AgedQueue::oldest (void *& reference)
{
    if (numRecords_ == 0) {
        return false;
    }

    t_AgeRecord *  currRecord = (t_AgeRecord *) &( (*recordList_)[oldestIndex_] );
    reference = currRecord->reference;

    return true;
}



bool
AgedQueue::next (void *& reference)
{
    if (!exists (reference)) {
        return false;
    }

    auto indexIter = ageIndex_->find (reference);
    t_Size currIndex = (*indexIter).second;

    t_AgeRecord *  currRecord = (t_AgeRecord *) &( (*recordList_)[currIndex] );

    if (currRecord->next == linkEnd) {
        // End of list...
        return false;
    }

    t_AgeRecord *  nextRecord = (t_AgeRecord *) &( (*recordList_)[currRecord->next] );
    reference = nextRecord->reference;

    
    return true;
}



bool
AgedQueue::prev (void *& reference)
{
    if (!exists (reference)) {
        return false;
    }

    auto indexIter = ageIndex_->find (reference);
    t_Size currIndex = (*indexIter).second;

    t_AgeRecord *  currRecord = (t_AgeRecord *) &( (*recordList_)[currIndex] );

    if (currRecord->prev == linkEnd) {
        // End of list...
        return false;
    }

    t_AgeRecord *  prevRecord = (t_AgeRecord *) &( (*recordList_)[currRecord->prev] );
    reference = prevRecord->reference;

    
    return true;
}



bool  
AgedQueue::resize (t_Size size)
{
    // Allocate new containers...
    // This method should only be called when the current containers are FULL
    //

    t_AgeRecordList *  newAgeRecordList = new t_AgeRecordList;
    newAgeRecordList->clear ();
    newAgeRecordList->resize (size);

    t_FreeIndexList *  newFreeIndexList = new t_FreeIndexList;
    newFreeIndexList->clear ();

    // Copy existing data...
    //
    t_Size i;
    for (i = 0; i < size_; i++) {
        (*newAgeRecordList)[i].reference = (*recordList_)[i].reference;
        (*newAgeRecordList)[i].prev = (*recordList_)[i].prev;
        (*newAgeRecordList)[i].next = (*recordList_)[i].next;
    }

    // Initialize remaining records...
    for (i = size_; i < size; i++) {
        newFreeIndexList->push_back (i);
        (*newAgeRecordList)[i].reference = nullptr;
        (*newAgeRecordList)[i].prev = linkEnd;
        (*newAgeRecordList)[i].next = linkEnd;
    }

    delete recordList_;
    delete freeIndexList_;
    size_ = size;

    recordList_ = newAgeRecordList;
    freeIndexList_ = newFreeIndexList;


    return true;
}



