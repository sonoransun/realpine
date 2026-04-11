/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>


template <class T>
struct OptHash
{
    size_t operator()(const T) const;
};


size_t OptStrHash(const char * str);

size_t OptDataHash(const byte * data, uint length);


template <>
size_t OptHash<string>::operator()(const string) const;

template <>
size_t OptHash<char *>::operator()(char *) const;

template <>
size_t OptHash<void *>::operator()(void *) const;

template <>
size_t OptHash<const char *>::operator()(const char *) const;

template <>
size_t OptHash<const void *>::operator()(const void *) const;

template <>
size_t OptHash<short>::operator()(const short) const;

template <>
size_t OptHash<ushort>::operator()(const ushort) const;

template <>
size_t OptHash<int>::operator()(const int) const;

template <>
size_t OptHash<uint>::operator()(const uint) const;

template <>
size_t OptHash<long>::operator()(const long) const;

template <>
size_t OptHash<ulong>::operator()(const ulong) const;

template <>
size_t OptHash<longlong>::operator()(const longlong) const;

template <>
size_t OptHash<ulonglong>::operator()(const ulonglong) const;
