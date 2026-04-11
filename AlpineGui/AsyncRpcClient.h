/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>


class AsyncRpcClient
{
  public:
    AsyncRpcClient() = default;
    ~AsyncRpcClient() = default;

    bool connect(const string & host, ushort port);

    void disconnect();

    bool isConnected() const;

    bool call(const string & method, const string & paramsJson, string & resultJson);


  private:
    string host_;
    ulong ipAddress_ = 0;
    ushort port_ = 0;
    ulong requestId_ = 1;
    bool connected_ = false;
};
