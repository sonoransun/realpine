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


#include <AlpineQueryModuleIndex.h>
#include <ReadLock.h>
#include <WriteLock.h>
#include <Log.h>
#include <StringUtils.h>



bool                                          AlpineQueryModuleIndex::initialized_s   = false;
AlpineQueryModuleIndex::t_OptionIdIndex *     AlpineQueryModuleIndex::optionIdIndex_s = nullptr;
AlpineQueryModuleIndex::t_ModuleIndex *       AlpineQueryModuleIndex::moduleIndex_s   = nullptr;
ReadWriteSem                                  AlpineQueryModuleIndex::dataLock_s;



bool  
AlpineQueryModuleIndex::initialize ()
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryModuleIndex::initialize invoked.");
#endif

    WriteLock  lock(dataLock_s);

    if (initialized_s) {
        Log::Error ("Attempt to reinitialize AlpineQueryModuleIndex!");
        return false;
    }
    optionIdIndex_s  = new t_OptionIdIndex;
    moduleIndex_s    = new t_ModuleIndex;

    initialized_s = true;


    return true;
}



bool  
AlpineQueryModuleIndex::registerQueryModule (AlpineQueryModule *  module)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryModuleIndex::registerQueryModule invoked.");
#endif

    WriteLock  lock(dataLock_s);

    if (!initialized_s) {
        Log::Error ("Call to AlpineQueryModuleIndex::registerQueryModule before "
                             "initialization of AlpineQueryModuleIndex!");
        return false;
    }
    // Get a list of all option IDs supported by this query module.
    //
    bool status;
    t_OptionIdList *  optionIdList;
    optionIdList = new t_OptionIdList;

    status = module->getQueryOptionExtensionList (*optionIdList);

    if (!status) {
        Log::Error ("Attempt to get queryOptionIdList from module failed in call to "
                             "AlpineQueryModuleIndex::registerQueryModule!");
        delete optionIdList;
        return false;
    }
    // Index by Option ID and module address
    //
    if (optionIdList->size ()) {
        for (auto& item : *optionIdList) {
            optionIdIndex_s->emplace (item, module);
        }
    }

    moduleIndex_s->emplace (reinterpret_cast<void *>(module), optionIdList);


    return true;
}



bool
AlpineQueryModuleIndex::removeQueryModule (AlpineQueryModule *  module)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryModuleIndex::removeQueryModule invoked.");
#endif

    WriteLock  lock(dataLock_s);

    if (!initialized_s) {
        Log::Error ("Call to AlpineQueryModuleIndex::removeQueryModule before "
                             "initialization of AlpineQueryModuleIndex!");
        return false;
    }
    // Attempt to locate the information associated with this module.
    //
    auto moduleIter = moduleIndex_s->find (reinterpret_cast<void *>(module));

    if (moduleIter == moduleIndex_s->end ()) {
        Log::Error ("Invalid module passed in call to "
                             "AlpineQueryModuleIndex::removeQueryModule!");
        return false;
    }
    t_OptionIdList *  optionIdList;
    optionIdList = (*moduleIter).second;

    if (optionIdList->size ()) {
        for (auto& item : *optionIdList) {
            optionIdIndex_s->erase (item);
        }
    }

    moduleIndex_s->erase (reinterpret_cast<void *>(module));
    delete optionIdList;


    return true;
}



bool  
AlpineQueryModuleIndex::processQuery (AlpineQueryRequest &                     queryRequest,
                                      AlpineQueryModule::t_ResourceDescList &  queryResults)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryModuleIndex::processQuery invoked.");
#endif

    queryResults.clear ();

    ReadLock  lock(dataLock_s);

    if (!initialized_s) {
        Log::Error ("Call to AlpineQueryModuleIndex::processQuery before "
                             "initialization of AlpineQueryModuleIndex!");
        return false;
    }
    // If the query has a protocol extension defined, locate the specific module
    // that supports this extension for processing.
    //
    // Otherwise, all modules receive generic queries with no extensions.
    //
    bool  status;
    ulong optionId = queryRequest.getOptionId ();

    if (optionId) {
        auto idIter = optionIdIndex_s->find (optionId);

        if (idIter == optionIdIndex_s->end ()) {
            Log::Error ("No module found for protocol extension option ID: "s +
                        std::to_string (optionId) + " in call to "
                        "AlpineQueryModuleIndex::processQuery!");
            return false;
        }
        AlpineQueryModule *  queryModule;
        queryModule = (*idIter).second;

        status = queryModule->processQuery (queryRequest, queryResults);

        return status;
    }


    // This is a generic query, with no extensions.  Send to all loaded query modules for
    // processing and aggregate results.
    //
    AlpineQueryModule *  currModule;
    AlpineQueryModule::t_ResourceDescList  currResults;

    for (auto& [key, value] : *moduleIndex_s) {

        currModule = reinterpret_cast<AlpineQueryModule *>(key);
        status = currModule->processQuery (queryRequest, currResults);

        if (!status) {
            Log::Error ("Processing query request failed for general query in call to "
                                 "AlpineQueryModuleIndex::processQuery!");
        }
        else {
            for (const auto& resultItem : currResults) {
                queryResults.push_back (resultItem);
            }

            currResults.clear ();
        }
    }


    return true;
}



