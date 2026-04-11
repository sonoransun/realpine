/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <DtcpBroadcast.h>


class AlpineQuery;


class AlpineBroadcast : public DtcpBroadcast
{
  public:
    AlpineBroadcast(DtcpBroadcastSet * destinations);

    virtual ~AlpineBroadcast();


    virtual bool handlePacketSend(ulong transportId);

    virtual bool handleSendComplete(ulong numSent, struct timeval & duration);


  private:
    AlpineQuery * query_;


    void setQueryParent(AlpineQuery * query);


    // Copy constructor and assignment operator not implemented
    //
    AlpineBroadcast(const AlpineBroadcast & copy);
    AlpineBroadcast & operator=(const AlpineBroadcast & copy);


    friend class AlpineQuery;
};
