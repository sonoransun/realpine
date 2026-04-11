/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>


class JsonRpcClient
{
  public:
    static bool initialize(const string & host, ushort port);

    static bool call(const string & method, const string & paramsJson, string & resultJson);


  private:
    static string host_s;
    static ulong ipAddress_s;
    static ushort port_s;
    static ulong requestId_s;
};
