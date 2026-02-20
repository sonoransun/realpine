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
#include <ReadWriteSem.h>
#include <OptHash.h>
#include <vector>


class AlpineQueryOptionData;
class AlpinePeerOptionData;
class AlpineProxyOptionData;
class AlpineExtensionModule;


class AlpineExtensionIndex
{
  public:


    static bool  initialize ();


    // Obtain various optional extension classes for use in demarshalling packet data, etc.
    //
    static bool  getQueryOptionExt (ulong                     optionId,
                                    AlpineQueryOptionData *&  optionData);

    static bool  getPeerOptionExt (ulong                    optionId,
                                   AlpinePeerOptionData *&  optionData);

    static bool  getProxyOptionExt (ulong                     optionId,
                                    AlpineProxyOptionData *&  optionData);

    static bool  exists (ulong  optionId);



    // Modules providing support for various extensions must register using these methods
    //
    static bool  registerExtensionModule (AlpineExtensionModule *  module);

    static bool  removeExtensionModule (AlpineExtensionModule *  module);



    // Private types 
    //
    using t_OptionIdList = vector<ulong>;

    struct t_ModuleOptionIdInfo {
        t_OptionIdList  queryIdList;
        t_OptionIdList  peerIdList;
        t_OptionIdList  proxyIdList;
    };

    using t_OptionIdIndex = std::unordered_map<ulong,
                     AlpineExtensionModule *,
                     OptHash<ulong>,
                     equal_to<ulong> >;

    using t_OptionIdIndexPair = std::pair<ulong, AlpineExtensionModule *>;


    using t_ModuleIndex = std::unordered_map<void *, // AlpineExtensionModule *
                     t_ModuleOptionIdInfo *,
                     OptHash<void *>,
                     equal_to<void *> >;

    using t_ModuleIndexPair = std::pair<void *, t_ModuleOptionIdInfo *>;



  private:

    static bool               initialized_s;
    static t_OptionIdIndex *  queryOptionIdIndex_s;
    static t_OptionIdIndex *  peerOptionIdIndex_s;
    static t_OptionIdIndex *  proxyOptionIdIndex_s;
    static t_ModuleIndex *    moduleIndex_s;
    static ReadWriteSem       dataLock_s;

};

