/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <DataBlock.h>


DataBlock::DataBlock(uint length)
    : buffer_(std::make_unique<byte[]>(length)),
      length_(length)
{}


DataBlock::DataBlock(DataBlock && other) noexcept
    : buffer_(std::move(other.buffer_)),
      length_(other.length_)
{
    other.length_ = 0;
}


DataBlock &
DataBlock::operator=(DataBlock && other) noexcept
{
    if (this != &other) {
        buffer_ = std::move(other.buffer_);
        length_ = other.length_;
        other.length_ = 0;
    }
    return *this;
}
