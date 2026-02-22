/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <DataBlock.h>



DataBlock::DataBlock (uint  length)
{
    buffer_ = new byte[length];
    length_ = length;
}



DataBlock::~DataBlock ()
{
    if (buffer_)
        delete [] buffer_;
}



