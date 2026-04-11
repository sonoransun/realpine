/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AlpinePeerLocation.h>
#include <AlpinePeerOptionData.h>
#include <Log.h>
#include <StringUtils.h>


AlpinePeerLocation::AlpinePeerLocation()
{
#ifdef _VERBOSE
    Log::Debug("AlpinePeerLocation constructor invoked.");
#endif

    ipAddress_ = 0;
    port_ = 0;
    optionId_ = 0;
    optionData_ = nullptr;
}


AlpinePeerLocation::AlpinePeerLocation(const AlpinePeerLocation & copy)
{
#ifdef _VERBOSE
    Log::Debug("AlpinePeerLocation copy constructor invoked.");
#endif

    ipAddress_ = copy.ipAddress_;
    port_ = copy.port_;
    optionId_ = copy.optionId_;

    if ((optionId_ != 0) && (copy.optionData_)) {
        optionData_ = copy.optionData_->duplicate();
    }
}


AlpinePeerLocation::~AlpinePeerLocation()
{
#ifdef _VERBOSE
    Log::Debug("AlpinePeerLocation destructor invoked.");
#endif

    delete optionData_;
}


AlpinePeerLocation &
AlpinePeerLocation::operator=(const AlpinePeerLocation & copy)
{
#ifdef _VERBOSE
    Log::Debug("AlpinePeerLocation assignment operator invoked.");
#endif

    if (&copy == this) {
        return *this;
    }

    ipAddress_ = copy.ipAddress_;
    port_ = copy.port_;
    optionId_ = copy.optionId_;

    if ((optionId_ != 0) && (copy.optionData_)) {
        if (optionData_) {
            delete optionData_;
        }

        optionData_ = copy.optionData_->duplicate();
    }


    return *this;
}


void
AlpinePeerLocation::setIpAddress(ulong ipAddress)
{
    ipAddress_ = ipAddress;
}


ulong
AlpinePeerLocation::getIpAddress()
{
    return ipAddress_;
}


void
AlpinePeerLocation::setPort(ushort port)
{
    port_ = port;
}


ushort
AlpinePeerLocation::getPort()
{
    return port_;
}


bool
AlpinePeerLocation::setOptionId(ulong optionId)
{
#ifdef _VERBOSE
    Log::Debug("AlpinePeerLocation::setOptionId invoked.");
#endif

    if (optionId != 0) {
        Log::Error("Attempt to set extended option ID in call to "
                   "AlpinePeerLocation::setOptionId!  Use setOptionData for extended options.");
        return false;
    }
    optionId_ = optionId;


    return true;
}


ulong
AlpinePeerLocation::getOptionId()
{
    return optionId_;
}


bool
AlpinePeerLocation::setOptionData(AlpinePeerOptionData * optionData)
{
#ifdef _VERBOSE
    Log::Debug("AlpinePeerLocation::setOptionData invoked.");
#endif

    if (optionData_) {
        delete optionData_;
    }

    optionId_ = optionData->getOptionId();
    optionData_ = optionData->duplicate();


    return true;
}


bool
AlpinePeerLocation::getOptionData(AlpinePeerOptionData *& optionData)
{
#ifdef _VERBOSE
    Log::Debug("AlpinePeerLocation::getOptionData invoked.");
#endif

    if ((!optionId_) || (!optionData_)) {
        Log::Error("Attempt to get option data when no extension set in call to "
                   "AlpinePeerLocation::getOptionData!");
        return false;
    }
    optionData = optionData_->duplicate();


    return true;
}
