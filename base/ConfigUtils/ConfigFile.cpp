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



