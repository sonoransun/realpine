/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>


class DataBlock
{
  public:

    DataBlock (uint  length);

    ~DataBlock ();


    byte *  buffer_;
    uint    length_;
};


