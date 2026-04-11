/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>


class AlpineProtocol
{
  public:
    AlpineProtocol() = default;
    ~AlpineProtocol() = default;


    // Various packet data limits
    //
    static const ulong maxQueryStringSize_s;
    static const ulong maxDescriptionStringSize_s;
    static const ulong maxResourceListSize_s;
    static const ulong maxPeerListSize_s;


    // Maximum number of consecutive duplicate packets before we consider maliscious intent.
    //
    static const ushort maxDuplicateRecv_s;
};
