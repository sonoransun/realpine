/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>


class DataBuffer;


class StackLinkInterface
{
  public:
    StackLinkInterface() = default;
    virtual ~StackLinkInterface();


    virtual bool setParent(StackLinkInterface * parent) = 0;

    virtual void unsetParent() = 0;

    virtual bool writeData(DataBuffer * linkBuffer) = 0;

    virtual bool readData(DataBuffer * linkBuffer) = 0;
};
