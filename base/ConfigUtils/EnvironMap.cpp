/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <EnvironMap.h>
#include <Log.h>
#include <StringUtils.h>

#include <cstring>


extern char ** environ;


EnvironMap::EnvironMap()
{
    envIndex_ = nullptr;
}


EnvironMap::~EnvironMap()
{
    delete envIndex_;
}


bool
EnvironMap::load()
{
#ifdef _VERBOSE
    Log::Debug("EnvironMap::load invoked.");
#endif

    if (!envIndex_) {
        envIndex_ = new t_EnvIndex;
    }

    char ** envVars = environ;

    const char * delimiter = "=";
    char * currKey;
    char * currValue;
    char * nextSubstr;
    char * currSet;


    // Parse env settings into key / value pairs and update envronmentMap_
    //
    const int buffSize = 2048;  // use maxmimum value to ensure enough space
    currSet = new char[buffSize];

    while (*envVars) {
        strncpy(currSet, *envVars, buffSize - 1);

        nextSubstr = nullptr;
        currKey = strtok_r(currSet, delimiter, &nextSubstr);

        if (!currKey) {
            envVars++;
            continue;
        }

        currValue = strtok_r(nullptr, delimiter, &nextSubstr);

        if (!currValue) {
            envVars++;
            continue;
        }

        envIndex_->insert(t_EnvIndexPair(std::string(currKey), std::string(currValue)));

        envVars++;
    }

    delete[] currSet;


    return true;
}


bool
EnvironMap::exists(const string & name)
{
#ifdef _VERBOSE
    Log::Debug("EnvironMap::exists invoked.");
#endif

    if (!envIndex_) {
        return false;
    }
    return envIndex_->find(name) != envIndex_->end();
}


bool
EnvironMap::get(const string & name, string & value)
{
#ifdef _VERBOSE
    Log::Debug("EnvironMap::get invoked.");
#endif

    if (!envIndex_) {
        return false;
    }
    auto iter = envIndex_->find(name);

    if (iter == envIndex_->end()) {
        return false;
    }
    value = (*iter).second;


    return true;
}
