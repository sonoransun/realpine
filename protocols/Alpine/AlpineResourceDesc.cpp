/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AlpineResourceDesc.h>
#include <AlpineQueryOptionData.h>
#include <AlpineExtensionIndex.h>
#include <Log.h>
#include <StringUtils.h>



AlpineResourceDesc::AlpineResourceDesc ()
{
#ifdef _VERBOSE
    Log::Debug ("AlpineResourceDesc constructor invoked.");
#endif

    matchId_     = 0;
    size_        = 0;
    locatorList_.clear ();
    description_ = "";
    optionId_    = 0;
    optionData_  = nullptr;
}



AlpineResourceDesc::AlpineResourceDesc (const AlpineResourceDesc & copy)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineResourceDesc copy constructor invoked.");
#endif

    matchId_     = copy.matchId_;
    size_        = copy.size_;
    locatorList_ = copy.locatorList_;
    description_ = copy.description_;
    optionId_    = copy.optionId_;

    if ( (optionId_ != 0) && (copy.optionData_) ) {
        optionData_ = copy.optionData_->duplicate ();
    }
}



AlpineResourceDesc::~AlpineResourceDesc ()
{
#ifdef _VERBOSE
    Log::Debug ("AlpineResourceDesc destructor invoked.");
#endif

    if (optionData_) {
        delete optionData_;
    }
}



AlpineResourceDesc & 
AlpineResourceDesc::operator = (const AlpineResourceDesc & copy)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineResourceDesc assignment invoked.");
#endif

    if (&copy == this) {
        return *this;
    }

    matchId_     = copy.matchId_;
    size_        = copy.size_;
    locatorList_ = copy.locatorList_;
    description_ = copy.description_;
    optionId_    = copy.optionId_;

    if ( (optionId_ != 0) && (copy.optionData_) ) {
        if (optionData_) {
            delete optionData_;
        }

        optionData_ = copy.optionData_->duplicate ();
    }


    return *this;
}



void
AlpineResourceDesc::setMatchId (ulong  matchId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineResourceDesc::setMatchId invoked.  Match ID: "s +
                std::to_string (matchId));
#endif

    matchId_ = matchId;
}



ulong  
AlpineResourceDesc::getMatchId ()
{
    return matchId_;
}



void  
AlpineResourceDesc::setSize (ulong  size)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineResourceDesc::setSize invoked.  Size: "s +
                std::to_string (size));
#endif

    size_ = size;
}



ulong  
AlpineResourceDesc::getSize ()
{
    return size_;
}



void  
AlpineResourceDesc::setLocatorList (const t_LocatorList &  locatorList)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineResourceDesc::setLocatorList invoked.");
#endif

    locatorList_ = locatorList;
}



void  
AlpineResourceDesc::getLocatorList (t_LocatorList &  locatorList)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineResourceDesc::getLocatorList invoked.");
#endif

    locatorList = locatorList_;
}



void  
AlpineResourceDesc::setDescription (const string &  description)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineResourceDesc::setDescription invoked.  Description: "s + description);
#endif

    description_ = description;
}



void  
AlpineResourceDesc::getDescription (string &  description)
{
    description = description_;
}



bool  
AlpineResourceDesc::setOptionId (ulong  optionId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineResourceDesc::setOptionId invoked.  Option ID: "s +
                std::to_string (optionId));
#endif

    if (optionId != 0) {
        Log::Error ("Attempt to set extended option ID in call to "
                             "AlpineResourceDesc::setOptionId!  Use setOptionData for extended options.");
        return false;
    }
    optionId_ = optionId;


    return true;
}



ulong  
AlpineResourceDesc::getOptionId ()
{
    return optionId_;
}



bool  
AlpineResourceDesc::setOptionData (AlpineQueryOptionData *  optionData)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineResourceDesc::setOptionData invoked.");
#endif

    if (optionData_) {
        delete optionData_;
    }

    optionId_   = optionData->getOptionId ();
    optionData_ = optionData->duplicate ();


    return true;
}



bool  
AlpineResourceDesc::getOptionData (AlpineQueryOptionData *&  optionData)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineResourceDesc::getOptionData invoked.");
#endif

    if ( (!optionId_) || (!optionData_) ) {
        Log::Error ("Attempt to get option data when no extension set in call to "
                             "AlpineResourceDesc::getOptionData!");
        return false;
    }
    optionData = optionData_->duplicate ();


    return true;
}



