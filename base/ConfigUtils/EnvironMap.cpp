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


#include <EnvironMap.h>
#include <Log.h>
#include <StringUtils.h>


extern char ** environ;



EnvironMap::EnvironMap ()
{
    envIndex_ = nullptr;
}



EnvironMap::~EnvironMap ()
{
    delete envIndex_;
}


 
bool  
EnvironMap::load ()
{
#ifdef _VERBOSE
    Log::Debug ("EnvironMap::load invoked.");
#endif

    if (!envIndex_) {
        envIndex_ =  new t_EnvIndex;
    }

    char **envVars = environ;

    const char *delimiter = "=";
    char *currKey;
    char *currValue;
    char *nextSubstr;
    char *currSet;


    // Parse env settings into key / value pairs and update envronmentMap_
    //
    const int buffSize = 2048;  // use maxmimum value to ensure enough space
    currSet = new char[buffSize];

    while (*envVars) {
        strncpy(currSet, *envVars, buffSize-1);

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

        envIndex_->insert( t_EnvIndexPair (std::string(currKey), std::string(currValue)) );

        envVars++;
    }

    delete [] currSet;
 

    return true;
}



bool  
EnvironMap::exists (const string & name)
{
#ifdef _VERBOSE
    Log::Debug ("EnvironMap::exists invoked.");
#endif

    if (!envIndex_) {
        return false;
    }
    return envIndex_->find (name) != envIndex_->end ();
}



bool
EnvironMap::get (const string & name,
                 string &       value)
{
#ifdef _VERBOSE
    Log::Debug ("EnvironMap::get invoked.");
#endif

    if (!envIndex_) {
        return false;
    }
    auto iter = envIndex_->find (name);

    if (iter == envIndex_->end ()) {
        return false;
    }
    value = (*iter).second;


    return true;
}



