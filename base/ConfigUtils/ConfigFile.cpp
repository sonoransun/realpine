/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <ConfigFile.h>
#include <Log.h>
#include <StringUtils.h>



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



