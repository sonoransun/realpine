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


#include <AlpineQueryOptions.h>
#include <AlpineGroupMgr.h>
#include <AlpineQueryOptionData.h>
#include <Log.h>
#include <StringUtils.h>



static const ulong   defaultHaltLimit    = 10;
static const ulong   defaultMaxDescLimit = 128;



AlpineQueryOptions::AlpineQueryOptions ()
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryOptions constructor invoked.");
#endif

    group_           = "default";
    autoHaltLimit_   = defaultHaltLimit;
    maxDescPerPeer_  = defaultMaxDescLimit;
    query_           = "";
    optionId_        = 0;
    optionData_      = nullptr;
}



AlpineQueryOptions::AlpineQueryOptions (const AlpineQueryOptions & copy)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryOptions copy constructor invoked.");
#endif

    group_           = copy.group_;
    autoHaltLimit_   = copy.autoHaltLimit_;
    maxDescPerPeer_  = copy.maxDescPerPeer_;
    query_           = copy.query_;
    optionId_        = copy.optionId_;

    if ( (optionId_ != 0) && (copy.optionData_) ) {
        optionData_ = copy.optionData_->duplicate ();
    }
}



AlpineQueryOptions::~AlpineQueryOptions ()
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryOptions destructor invoked.");
#endif

    if (optionData_) {
        delete optionData_;
        optionData_ = nullptr;
    }
}



const AlpineQueryOptions & 
AlpineQueryOptions::operator = (const AlpineQueryOptions & copy)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryOptions assignment invoked.");
#endif

    if (&copy == this) {
        return *this;
    }

    group_           = copy.group_;
    autoHaltLimit_   = copy.autoHaltLimit_;
    maxDescPerPeer_  = copy.maxDescPerPeer_;
    query_           = copy.query_;
    optionId_        = copy.optionId_;

    if ( (optionId_ != 0) && (copy.optionData_) ) {
        if (optionData_) {
            delete optionData_;
        }

        optionData_ = copy.optionData_->duplicate ();
    }

    return *this;
}



bool  
AlpineQueryOptions::setGroup (const string &  groupName)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryOptions::setGroup invoked.  Group name: "s + groupName);
#endif

    bool status;

    // Verify that this is a valid group name.
    //
    status = AlpineGroupMgr::exists (groupName);

    if (!status) {
        Log::Error ("Invalid group name passed to AlpineQueryOptions::setGroup!");
        return false;
    }
    group_ = groupName;


    return true;
}



bool  
AlpineQueryOptions::getGroup (string &  groupName)
{
    groupName = group_;

    return true;
}



bool  
AlpineQueryOptions::setAutoHalt (ulong  numHits)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryOptions::setAutoHalt invoked.  AutoHalt threshold: "s +
                std::to_string (numHits));
#endif

    autoHaltLimit_ = numHits;

    return true;
}



bool  
AlpineQueryOptions::getAutoHalt (ulong &  numHits)
{
    numHits = autoHaltLimit_;

    return true;
}



bool  
AlpineQueryOptions::setMaxDescPerPeer (ulong  maxDesc)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryOptions::setMaxDescPerPeer invoked.  MaxDesc threshold: "s +
                std::to_string (maxDesc));
#endif
  
    maxDescPerPeer_ = maxDesc;
    
    return true;
}



bool  
AlpineQueryOptions::getMaxDescPerPeer (ulong &  maxDesc)
{
    maxDesc = maxDescPerPeer_;

    return true;
}



bool  
AlpineQueryOptions::setQuery (const string &  query)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryOptions::setQuery invoked.  Query: "s + query);
#endif

    // Query has to be non null
    // MRP_TEMP define constants in AlpineLimits:: and verify
    //
    if (query.empty()) {
        Log::Error ("Invalid query passed to AlpineQueryOptions::setQuery!");
        return false;
    }
    query_ = query;

    return true;
}



bool  
AlpineQueryOptions::getQuery (string &  query)
{
    query = query_;

    return true;
}



bool  
AlpineQueryOptions::setOptionId (ulong  optionId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryOptions::setOptionId invoked.  Value: "s +
                std::to_string (optionId));
#endif

    if (optionId != 0) {
        Log::Error ("Attempt to set extended option ID in call to "
                             "AlpineQueryOptions::setOptionId!  Use setOptionData for extended options.");
        return false;
    }
    optionId_ = optionId;

    return true;
}



bool  
AlpineQueryOptions::getOptionId (ulong &  optionId)
{
    optionId = optionId_;

    return true;
}



bool  
AlpineQueryOptions::setOptionData (AlpineQueryOptionData *  optionData)
{
#ifdef _VERBOSE
    Log::Debug (" invoked.");
#endif

    if (optionData_) {
        delete optionData_;
        optionData_ = nullptr;
    }

    optionId_   = optionData->getOptionId ();
    optionData_ = optionData->duplicate ();


    return true;
}



bool
AlpineQueryOptions::getOptionData (AlpineQueryOptionData *&  optionData)
{
#ifdef _VERBOSE
    Log::Debug (" invoked.");
#endif

    if ( (optionId_ == 0) || (!optionData_) ) {
        Log::Error ("Attempt to get option data when no extension set in call to "
                             "AlpineQueryOptions::getOptionData!");
        return false;
    }
    optionData = optionData_->duplicate ();

    return true;
}



