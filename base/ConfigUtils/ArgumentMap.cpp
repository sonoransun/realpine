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


#include <ArgumentMap.h>
#include <Log.h>
#include <StringUtils.h>



ArgumentMap::ArgumentMap ()
{
    argIndex_ = nullptr;
}



ArgumentMap::~ArgumentMap ()
{
    delete argIndex_;
}


  
bool  
ArgumentMap::load (int     argc,
                   char ** argv)
{
#ifdef _VERBOSE
    Log::Debug ("ArgumentMap::load invoked"s +
                "\nArg Count: "s + std::to_string(argc) +
                "\n");
#endif

    if (!argIndex_) {
        argIndex_ = new t_ArgIndex;
    }

    if (argc <= 0) {
        // nothing to parse
        return true;
    }
    // obtain all arguments passed in argv list.
    //
    // Iterate through args, if prefixed with a '-' or '--',
    // use this as the option name.  Any following value without
    // a '-' or '--' prefix is considered an option value, otherwise
    // the option is present, but has no associated value. (perfect sense eh?)
    //

    char * optionName = nullptr;
    char * currArg;
    char ** currArgPtr;
    char * lastArg;

    currArgPtr = argv;
    currArg = *(currArgPtr++);
    lastArg = argv[argc - 1];
    

    bool finished = false;

    while (!finished) {

        // see if this is an option
        //
        if (*currArg == '-') {

            if (optionName) {
                // the previous option has no value, insert into the index,
                // update the optionName to this new option.
                //
                argIndex_->emplace (std::string(optionName), "");

#ifdef _VERBOSE
                Log::Debug ("Added null option name: "s + std::string(optionName));
#endif

                if (*(currArg+1) == '-') {
                    optionName = currArg + 2;
                }
                else {
                    optionName = currArg + 1;
                }
            }
            else {
                if (*(currArg+1) == '-') {
                    optionName = currArg + 2;
                }
                else {
                    optionName = currArg + 1;
                }
            }

            // finished processing this token...
            //
            if (currArg == lastArg) {
                finished = true;
            }
            else {
                currArg = *(currArgPtr++);
            }

            continue;
        }

        // This is a value.  If we do not have a preceeding option name, ignore.
        //
        if (optionName) {

            // add to index and reset optionName...
            //
            argIndex_->emplace (std::string(optionName), std::string(currArg));

#ifdef _VERBOSE
            Log::Debug ("Added option: "s + std::string(optionName) + 
                        "="s + std::string(currArg));
#endif

            optionName = nullptr;
        }

        // finished processing this token...
        //
        if (currArg == lastArg) {
            finished = true;
        }
        else {
            currArg = *(currArgPtr++);
        }
    }


    return true;
}



bool  
ArgumentMap::exists (const string & name)
{
#ifdef _VERBOSE
    Log::Debug ("ArgumentMap::exists invoked for option name: "s + name);
#endif

    if (!argIndex_) {
        return false;
    }
    return argIndex_->find (name) != argIndex_->end ();
}



bool
ArgumentMap::get (const string & name,
                  string &       value)
{
#ifdef _VERBOSE
    Log::Debug ("ArgumentMap::get invoked for option name: "s + name);
#endif

    if (!argIndex_) {
        return false;
    }
    auto iter = argIndex_->find (name);

    if (iter == argIndex_->end ()) {
        // nope...
        return false;
    }
    value = (*iter).second;


    return true;
}



