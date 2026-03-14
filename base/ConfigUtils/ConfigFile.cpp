/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <ConfigFile.h>
#include <Log.h>
#include <StringUtils.h>
#include <Platform.h>

#ifdef ALPINE_PLATFORM_POSIX
#include <sys/stat.h>
#endif



ConfigFile::ConfigFile ()
{
    nameIndex_ = nullptr;
    isDirty_   = false;
}



ConfigFile::~ConfigFile ()
{
    delete nameIndex_;
}



bool  
ConfigFile::initialize (const string & fileName)
{
#ifdef _VERBOSE
    Log::Debug ("ConfigFile::initialize invoked.");
#endif

    fileName_ = fileName;

#ifdef ALPINE_PLATFORM_POSIX
    struct stat fileStat;
    if (stat(fileName_.c_str(), &fileStat) == 0) {
        if (fileStat.st_mode & (S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)) {
            Log::Error("WARNING: Config file '"s + fileName_ + "' has insecure permissions. Recommend chmod 600."s);
        }
    }
#endif

    return true;
}



bool  
ConfigFile::exists (const string & name)
{
#ifdef _VERBOSE
    Log::Debug ("ConfigFile::exists invoked.");
#endif


    return true;
}



bool  
ConfigFile::get (const string & name,
                 string &       value)
{
#ifdef _VERBOSE
    Log::Debug ("ConfigFile::get invoked.");
#endif


    return true;
}



bool  
ConfigFile::set (const string & name,
                 const string & value)
{
#ifdef _VERBOSE
    Log::Debug ("ConfigFile::set invoked.");
#endif


    return true;
}



bool  
ConfigFile::save ()
{
#ifdef _VERBOSE
    Log::Debug ("ConfigFile::save invoked.");
#endif


    return true;
}



