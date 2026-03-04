/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <memory>


class DataBlock
{
  public:

    DataBlock (uint  length);

    ~DataBlock () noexcept = default;

    DataBlock (DataBlock && other) noexcept;
    DataBlock & operator= (DataBlock && other) noexcept;

    DataBlock (const DataBlock &) = delete;
    DataBlock & operator= (const DataBlock &) = delete;


    std::unique_ptr<byte[]>  buffer_;
    uint                     length_;
};


