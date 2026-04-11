/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AlpineQueryOptionData.h>
#include <AlpineQueryRequest.h>
#include <Log.h>
#include <StringUtils.h>


AlpineQueryRequest::AlpineQueryRequest()
{
    queryId_ = 0;
    queryString_ = "";
    optionId_ = 0;
    optionData_ = nullptr;
}


AlpineQueryRequest::AlpineQueryRequest(const AlpineQueryRequest & copy)
{
#ifdef _VERBOSE
    Log::Debug("AlpineQueryRequest copy constructor invoked.");
#endif

    queryId_ = copy.queryId_;
    queryString_ = copy.queryString_;
    optionId_ = copy.optionId_;

    if (optionId_) {
        optionData_ = copy.optionData_->duplicate();
    }
}


AlpineQueryRequest::~AlpineQueryRequest()
{
    if (optionData_)
        delete (optionData_);
}


AlpineQueryRequest &
AlpineQueryRequest::operator=(const AlpineQueryRequest & copy)
{
#ifdef _VERBOSE
    Log::Debug("AlpineQueryRequest assignment operator invoked.");
#endif

    if (&copy == this) {
        return *this;
    }

    if (optionData_) {
        delete optionData_;
        optionData_ = nullptr;
    }

    queryId_ = copy.queryId_;
    queryString_ = copy.queryString_;
    optionId_ = copy.optionId_;

    if (optionId_) {
        optionData_ = copy.optionData_->duplicate();
    }


    return *this;
}


bool
AlpineQueryRequest::setQueryId(ulong queryId)
{
#ifdef _VERBOSE
    Log::Debug("AlpineQueryRequest::setQueryId invoked.  Query ID: "s + std::to_string(queryId));
#endif

    queryId_ = queryId;

    return true;
}


ulong
AlpineQueryRequest::getQueryId()
{
    return queryId_;
}


bool
AlpineQueryRequest::setQueryString(const string & queryString)
{
#ifdef _VERBOSE
    Log::Debug("AlpineQueryRequest::setQueryString invoked.  Query: "s + queryString);
#endif

    queryString_ = queryString;

    return true;
}


string
AlpineQueryRequest::getQueryString()
{
    return queryString_;
}


bool
AlpineQueryRequest::setOptionId(ulong optionId)
{
#ifdef _VERBOSE
    Log::Debug("AlpineQueryRequest::setOptionId invoked.  Option ID: "s + std::to_string(optionId));
#endif

    if (optionId != 0) {
        Log::Error("Attempt to set extended option ID in call to "
                   "AlpineQueryRequest::setOptionId!  Use setOptionData for extended options.");
        return false;
    }
    optionId_ = optionId;


    return true;
}


ulong
AlpineQueryRequest::getOptionId()
{
    return optionId_;
}


bool
AlpineQueryRequest::setOptionData(AlpineQueryOptionData * optionData)
{
#ifdef _VERBOSE
    Log::Debug("AlpineQueryRequest::setOptionDat invoked.");
#endif

    if (optionData_) {
        delete optionData_;
        optionData_ = nullptr;
    }

    optionId_ = optionData->getOptionId();
    optionData_ = optionData->duplicate();


    return true;
}


bool
AlpineQueryRequest::getOptionData(AlpineQueryOptionData *& optionData)
{
#ifdef _VERBOSE
    Log::Debug("AlpineQueryRequest::getOptionData invoked.");
#endif

    if ((optionId_ == 0) || (!optionData_)) {
        Log::Error("Attempt to get option data when no extension set in call to "
                   "AlpineQueryRequest::getOptionData!");
        return false;
    }
    optionData = optionData_->duplicate();


    return true;
}
