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
#include <AlpineQueryPacket.h>
#include <AlpineResourceDesc.h>
#include <vector>


class AlpineResourceDescSet
{
  public:

    AlpineResourceDescSet ();

    AlpineResourceDescSet (const AlpineResourceDescSet & copy);

    ~AlpineResourceDescSet ();

    AlpineResourceDescSet & operator = (const AlpineResourceDescSet & copy);



    using t_ResourceDescList = vector<AlpineResourceDesc>;



    bool   clear ();

    bool   reserve (ulong  size);  // if the expected size is know, provide a hint

    // These methods track data from query reply packets
    // 
    bool   getResourceDataPacket (AlpineQueryPacket *  queryPacket);

    bool   addResourceDataPacket (AlpineQueryPacket *  queryPacket);

    ulong  size ();

    bool   getCurrOffset (ulong &  offset);

    bool   getRemaining (ulong &  numRemaining);
 
    bool   addResource (AlpineResourceDesc &  resource);

    bool   addResourceList (t_ResourceDescList &  resourceList);

    bool   getResourceList (t_ResourceDescList &  resourceList);



    // Internal types
    //
    using t_ResourceDescArray = vector<AlpineResourceDesc *>;


  private:

    ulong                   arraySize_;
    ulong                   numResources_;
    ulong                   currOffset_;
    t_ResourceDescArray *   resourceList_;



    static ulong getResourceDescSize (AlpineResourceDesc *  desc);

    bool  resize (ulong  newSize);
};


