/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <StackLinkInterface.h>


class DataBuffer;


class DtcpDataPacket : public StackLinkInterface
{
  public:

    DtcpDataPacket ();

    DtcpDataPacket (StackLinkInterface * parent);

    virtual ~DtcpDataPacket ();



    ////
    //
    // StackLink operations
    //
    virtual bool  setParent (StackLinkInterface *  parent);

    virtual void  unsetParent ();

    virtual bool  writeData (DataBuffer * linkBuffer);

    virtual bool  readData (DataBuffer * linkBuffer);


    
    ////
    //
    // Data packet operations
    //
    virtual bool  setPacketData (const byte * data,
                                 const uint   dataLength);

    virtual bool  getPacketData (byte *& data,
                                 uint &  dataLength);


  protected:

    StackLinkInterface * parent_;
    const byte *         setData_;
    uint                 setDataLength_;
    byte *               getData_;
    uint                 getDataLength_;

};


