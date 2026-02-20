///////
///
///  Copyright (C) 2026  sonoransun
///
///  Permission is hereby granted, free of charge, to any person obtaining a copy
///  of this software and associated documentation files (the "Software"), to deal
///  in the Software without restriction, including without limitation the rights
///  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
///  copies of the Software, and to permit persons to whom the Software is
///  furnished to do so, subject to the following conditions:
///
///  The above copyright notice and this permission notice shall be included in all
///  copies or substantial portions of the Software.
///
///  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
///  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
///  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
///  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
///  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
///  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
///  SOFTWARE.
///
///////


#pragma once
#include <Common.h>


template<class T>
struct OptHash
{
    size_t operator () (const T) const;
};


size_t
OptStrHash (const char * str);

size_t
OptDataHash (const byte *  data,
             uint          length);


template <>
size_t
OptHash<string>::operator () (const string) const;

template <>
size_t
OptHash<char *>::operator () (char *) const;

template <>
size_t
OptHash<void *>::operator () (void *) const;

template <>
size_t
OptHash<const char *>::operator () (const char *) const;

template <>
size_t
OptHash<const void *>::operator () (const void *) const;

template <>
size_t
OptHash<short>::operator () (const short) const;

template <>
size_t
OptHash<ushort>::operator () (const ushort) const;

template <>
size_t
OptHash<int>::operator () (const int) const;

template <>
size_t
OptHash<uint>::operator () (const uint) const;

template <>
size_t
OptHash<long>::operator () (const long) const;

template <>
size_t
OptHash<ulong>::operator () (const ulong) const;

template <>
size_t
OptHash<longlong>::operator () (const longlong) const;

template <>
size_t
OptHash<ulonglong>::operator () (const ulonglong) const;




