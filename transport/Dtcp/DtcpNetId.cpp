/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <DtcpNetId.h>


void 
netIdToIpPort (const t_NetId &  netId,
               ulong &          ipAddress,
               ushort &         port)
{
    ipAddress = ulong (netId >> 32);
    port      = ushort (netId);
}



void 
ipPortToNetId (const ulong   ipAddress,
               const ushort  port,
               t_NetId &     netId)
{
    netId = ipAddress;
    netId <<= 32;
    netId += port;
}



