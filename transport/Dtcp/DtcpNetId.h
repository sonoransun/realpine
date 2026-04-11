/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>


////
//
// A Net ID is the ip address and port of an
// endpoint.  This is commonly called a 'socket'
// however, netID is used to prevent confusion
// with the already overused 'socket' term.
//
// The IP Address is stored in the high order word,
// the port in the low.  (wasting 16 bytes...)
//

using t_NetId = ulonglong;

void netIdToIpPort(const t_NetId & netId, ulong & ipAddress, ushort & port);

void ipPortToNetId(const ulong ipAddress, const ushort port, t_NetId & netId);
