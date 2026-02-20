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


#include <AlpineExtensionIndex.h>
#include <AlpineQueryOptionData.h>
#include <AlpinePeerOptionData.h>
#include <AlpineProxyOptionData.h>
#include <AlpineExtensionModule.h>
#include <ReadLock.h>
#include <WriteLock.h>
#include <Log.h>
#include <StringUtils.h>



bool                                         AlpineExtensionIndex::initialized_s         = false;
AlpineExtensionIndex::t_OptionIdIndex *      AlpineExtensionIndex::queryOptionIdIndex_s  = nullptr;
AlpineExtensionIndex::t_OptionIdIndex *      AlpineExtensionIndex::peerOptionIdIndex_s   = nullptr;
AlpineExtensionIndex::t_OptionIdIndex *      AlpineExtensionIndex::proxyOptionIdIndex_s  = nullptr;
AlpineExtensionIndex::t_ModuleIndex *        AlpineExtensionIndex::moduleIndex_s         = nullptr;
ReadWriteSem                                 AlpineExtensionIndex::dataLock_s;



bool  
AlpineExtensionIndex::initialize ()
{
#ifdef _VERBOSE
    Log::Debug ("AlpineExtensionIndex::initialize invoked.");
#endif

    WriteLock  lock(dataLock_s);

    if (initialized_s) {
        Log::Error ("Attempt to reinitialize AlpineExtensionIndex!");
        return false;
    }
    queryOptionIdIndex_s  = new t_OptionIdIndex;
    peerOptionIdIndex_s   = new t_OptionIdIndex;
    proxyOptionIdIndex_s  = new t_OptionIdIndex;
    moduleIndex_s         = new t_ModuleIndex;

    initialized_s = true;


    return true;
}



bool  
AlpineExtensionIndex::getQueryOptionExt (ulong                     optionId,
                                         AlpineQueryOptionData *&  optionData)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineExtensionIndex::getQueryOptionExt invoked.  Option ID: "s +
                std::to_string (optionId));
#endif

    ReadLock  lock(dataLock_s);

    if (!initialized_s) {
        Log::Error ("Call to AlpineExtensionIndex::getQueryOptionExt before "
                             "initialization of AlpineExtensionIndex!");
        return false;
    }
    // Locate extension module to create OptionData class for this Option ID.
    //
    auto iter = queryOptionIdIndex_s->find (optionId);

    if (iter == queryOptionIdIndex_s->end ()) {
        Log::Error ("Invalid Option ID passed in call to "
                             "AlpineExtensionIndex::getQueryOptionExt!");
        return false;
    }
    bool status;
    AlpineExtensionModule *  extensionModule;
    extensionModule = (*iter).second;

    status = extensionModule->createQueryOptionData (optionId, optionData);

    if (!status) {
        Log::Error ("Attempt to create query option data failed in call to "
                             "AlpineExtensionIndex::getQueryOptionExt!");
        return false;
    }
    return true;
}



bool  
AlpineExtensionIndex::getPeerOptionExt (ulong                    optionId,
                                        AlpinePeerOptionData *&  optionData)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineExtensionIndex::getPeerOptionExt invoked.  Option ID: "s +
                std::to_string (optionId));
#endif

    ReadLock  lock(dataLock_s);

    if (!initialized_s) {
        Log::Error ("Call to AlpineExtensionIndex::getPeerOptionExt before "
                             "initialization of AlpineExtensionIndex!");
        return false;
    }
    // Locate extension module to create OptionData class for this Option ID.
    //
    auto iter = peerOptionIdIndex_s->find (optionId);

    if (iter == peerOptionIdIndex_s->end ()) {
        Log::Error ("Invalid Option ID passed in call to "
                             "AlpineExtensionIndex::getPeerOptionExt!");
        return false;
    }
    bool status;
    AlpineExtensionModule *  extensionModule;
    extensionModule = (*iter).second;

    status = extensionModule->createPeerOptionData (optionId, optionData);

    if (!status) {
        Log::Error ("Attempt to create query option data failed in call to "
                             "AlpineExtensionIndex::getPeerOptionExt!");
        return false;
    }
    return true;
}



bool  
AlpineExtensionIndex::getProxyOptionExt (ulong                     optionId,
                                         AlpineProxyOptionData *&  optionData)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineExtensionIndex::getProxyOptionExt invoked.  Option ID: "s +
                std::to_string (optionId));
#endif

    ReadLock  lock(dataLock_s);

    if (!initialized_s) {
        Log::Error ("Call to AlpineExtensionIndex::getProxyOptionExt before "
                             "initialization of AlpineExtensionIndex!");
        return false;
    }
    // Locate extension module to create OptionData class for this Option ID.
    //
    auto iter = proxyOptionIdIndex_s->find (optionId);

    if (iter == proxyOptionIdIndex_s->end ()) {
        Log::Error ("Invalid Option ID passed in call to "
                             "AlpineExtensionIndex::getProxyOptionExt!");
        return false;
    }
    bool status;
    AlpineExtensionModule *  extensionModule;
    extensionModule = (*iter).second;

    status = extensionModule->createProxyOptionData (optionId, optionData);

    if (!status) {
        Log::Error ("Attempt to create query option data failed in call to "
                             "AlpineExtensionIndex::getProxyOptionExt!");
        return false;
    }
    return true;
}



bool
AlpineExtensionIndex::exists (ulong  optionId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineExtensionIndex::exists invoked.  Option ID: "s +
                std::to_string (optionId));
#endif

    ReadLock  lock(dataLock_s);

    if (!initialized_s) {
        Log::Error ("Call to AlpineExtensionIndex::exists before "
                             "initialization of AlpineExtensionIndex!");
        return false;
    }
    // Attempt to locate extension module for this Option ID.
    //
    return proxyOptionIdIndex_s->find (optionId) != proxyOptionIdIndex_s->end ();
}



bool  
AlpineExtensionIndex::registerExtensionModule (AlpineExtensionModule *  module)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineExtensionIndex::registerExtensionModule invoked.");
#endif

    WriteLock  lock(dataLock_s);

    if (!initialized_s) {
        Log::Error ("Call to AlpineExtensionIndex::registerExtensionModule before "
                             "initialization of AlpineExtensionIndex!");
        return false;
    }
    // Get a list of all option IDs supported by this module.  Add to indexes for OptionID and
    // Module address
    //
    t_ModuleOptionIdInfo *  moduleInfo;
    moduleInfo = new t_ModuleOptionIdInfo;

    bool status;
    AlpineExtensionModule::t_OptionIdList  optionIdList;


    // Query Options
    //
    status = module->getQueryOptionExtensionList (optionIdList);

    if (!status) {
        Log::Error ("Attempt to get queryOptionIdList from module failed in call to "
                             "AlpineExtensionIndex::registerExtensionModule!");
        delete moduleInfo;
        return false;
    }
    if (optionIdList.size ()) {
        moduleInfo->queryIdList = optionIdList;
    }


    // Peer List Options
    //
    status = module->getPeerOptionExtensionList (optionIdList);

    if (!status) {
        Log::Error ("Attempt to get peerOptionIdList from module failed in call to "
                             "AlpineExtensionIndex::registerExtensionModule!");
        delete moduleInfo;
        return false;
    }
    if (optionIdList.size ()) {
        moduleInfo->peerIdList = optionIdList;
    }


    // Proxy Options
    //
    status = module->getProxyOptionExtensionList (optionIdList);

    if (!status) {
        Log::Error ("Attempt to get proxyOptionIdList from module failed in call to "
                             "AlpineExtensionIndex::registerExtensionModule!");
        delete moduleInfo;
        return false;
    }
    if (optionIdList.size ()) {
        moduleInfo->proxyIdList = optionIdList;
    }


    // Index by Option ID and module address
    //
    if (moduleInfo->queryIdList.size ()) {
        for (auto& item : moduleInfo->queryIdList) {
            queryOptionIdIndex_s->emplace (item, module);
        }
    }

    if (moduleInfo->peerIdList.size ()) {
        for (auto& item : moduleInfo->peerIdList) {
            peerOptionIdIndex_s->emplace (item, module);
        }
    }

    if (moduleInfo->proxyIdList.size ()) {
        for (auto& item : moduleInfo->proxyIdList) {
            proxyOptionIdIndex_s->emplace (item, module);
        }
    }

    moduleIndex_s->emplace (reinterpret_cast<void *>(module), moduleInfo);


    return true;
}



bool
AlpineExtensionIndex::removeExtensionModule (AlpineExtensionModule *  module)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineExtensionIndex::removeExtensionModule invoked.");
#endif

    WriteLock  lock(dataLock_s);

    if (!initialized_s) {
        Log::Error ("Call to AlpineExtensionIndex::removeExtensionModule before "
                             "initialization of AlpineExtensionIndex!");
        return false;
    }
    // Attempt to locate the information associated with this module.
    //
    auto moduleIter = moduleIndex_s->find (reinterpret_cast<void *>(module));

    if (moduleIter == moduleIndex_s->end ()) {
        Log::Error ("Invalid module passed in call to AlpineExtensionIndex::removeExtensionModule!");
        return false;
    }
    t_ModuleOptionIdInfo *  moduleInfo;
    moduleInfo = (*moduleIter).second;

    if (moduleInfo->queryIdList.size ()) {
        for (auto& item : moduleInfo->queryIdList) {
            queryOptionIdIndex_s->erase (item);
        }
    }

    if (moduleInfo->peerIdList.size ()) {
        for (auto& item : moduleInfo->peerIdList) {
            peerOptionIdIndex_s->erase (item);
        }
    }

    if (moduleInfo->proxyIdList.size ()) {
        for (auto& item : moduleInfo->proxyIdList) {
            proxyOptionIdIndex_s->erase (item);
        }
    }

    moduleIndex_s->erase (reinterpret_cast<void *>(module));
    delete moduleInfo;


    return true;
}



