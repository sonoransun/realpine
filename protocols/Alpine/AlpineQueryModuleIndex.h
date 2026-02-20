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


#pragma once
#include <Common.h>
#include <AlpineQueryModule.h>
#include <ReadWriteSem.h>
#include <OptHash.h>
#include <vector>


class AlpineQueryModuleIndex
{
  public:

    static bool  initialize ();

    static bool  registerQueryModule (AlpineQueryModule *  module);

    static bool  removeQueryModule (AlpineQueryModule *  module);

    // The processed query is dispatched to all modules if no extensions are given,
    // otherwise dispatch only to respective module which supports the extension.
    //
    static bool  processQuery (AlpineQueryRequest &                     queryRequest,
                               AlpineQueryModule::t_ResourceDescList &  queryResults);



    // Private types 
    //
    using t_OptionIdList = vector<ulong>;

    using t_OptionIdIndex = std::unordered_map<ulong,
                     AlpineQueryModule *,
                     OptHash<ulong>,
                     equal_to<ulong> >;

    using t_OptionIdIndexPair = std::pair<ulong, AlpineQueryModule *>;


    using t_ModuleIndex = std::unordered_map<void *, // AlpineQueryModule *
                     t_OptionIdList *,
                     OptHash<void *>,
                     equal_to<void *> >;

    using t_ModuleIndexPair = std::pair<void *, t_OptionIdList *>;



  private:

    static bool               initialized_s;
    static t_OptionIdIndex *  optionIdIndex_s;
    static t_ModuleIndex *    moduleIndex_s;
    static ReadWriteSem       dataLock_s;

};

