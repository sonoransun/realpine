/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <OptHash.h>


static void
OptHashMasher(ulong a, ulong b, ulong c)
{
    // Bastardized MIXer hash by some guy named bob i think...
    // See: http://burtleburtle.net/bob/hash/doobs.html
    //

    // "round and round and do se do..."
    //
    a -= b;
    a -= c;
    a ^= (c >> 13);
    b -= c;
    b -= a;
    b ^= (a << 8);
    c -= a;
    c -= b;
    c ^= (b >> 13);
    a -= b;
    a -= c;
    a ^= (c >> 12);
    b -= c;
    b -= a;
    b ^= (a << 16);
    c -= a;
    c -= b;
    c ^= (b >> 5);
    a -= b;
    a -= c;
    a ^= (c >> 3);
    b -= c;
    b -= a;
    b ^= (a << 10);
    c -= a;
    c -= b;
    c ^= (b >> 15);
}


static size_t
OptHashCharMasher(const char * str)
{
    // Here in lie the secrets of the universe.
    ulong a = 0x9e3272c5;
    ulong b = 0x6a22b5a1;
    ulong c = 0x23a4511b;

    const char * curr = str;

    int shift = 0;

    while (*curr) {
        if (shift > 3)
            shift = 0;

        a += *curr++ << (8 * shift);

        if (*curr) {
            b += *curr++ << (8 * shift);

            if (*curr) {
                c += *curr++ << (8 * shift);
            }
        }
    }

    OptHashMasher(a, b, c);

    return ((size_t)c);
}


size_t
OptStrHash(const char * str)
{
    return (OptHashCharMasher(str));
}


size_t
OptDataHash(const byte * data, uint length)
{
    ulong a = 0x9e3272c5;
    ulong b = 0x6a22b5a1;
    ulong c = 0x23a4511b;

    const byte * curr = data;

    int shift = 0;

    while ((uint)(curr - data) < length) {
        if (shift > 3)
            shift = 0;

        a += *curr++ << (8 * shift);

        if ((uint)(curr - data) < length) {
            b += *curr++ << (8 * shift);

            if ((uint)(curr - data) < length) {
                c += *curr++ << (8 * shift);
            }
        }
    }

    OptHashMasher(a, b, c);

    return ((size_t)c);
}


template <>
size_t
OptHash<string>::operator()(const string str) const
{
    return (OptHashCharMasher(str.c_str()));
}


template <>
size_t
OptHash<char *>::operator()(char * str) const
{
    return (OptHashCharMasher(str));
}


template <>
size_t
OptHash<void *>::operator()(void * val) const
{
    ulong a = 0x9e3272c5 ^ ((ulong)val << 16);
    ulong b = 0x6a22b5a1 ^ ((ulong)val >> 16);
    ulong c = 0x23a4511b ^ (ulong)val;

    OptHashMasher(a, b, c);

    return ((size_t)c);
}


template <>
size_t
OptHash<const char *>::operator()(const char * str) const
{
    return (OptHashCharMasher(str));
}


template <>
size_t
OptHash<const void *>::operator()(const void * val) const
{
    ulong a = 0x9e3272c5 ^ ((ulong)val << 16);
    ulong b = 0x6a22b5a1 ^ ((ulong)val >> 16);
    ulong c = 0x23a4511b ^ (ulong)val;

    OptHashMasher(a, b, c);

    return ((size_t)c);
}


template <>
size_t
OptHash<short>::operator()(const short val) const
{
    ulong a = 0x9e3272c5 ^ (val << 20);
    ulong b = 0x6a22b5a1 ^ (val << 10);
    ulong c = 0x23a4511b ^ val;

    OptHashMasher(a, b, c);

    return ((size_t)c);
}


template <>
size_t
OptHash<ushort>::operator()(const ushort val) const
{
    ulong a = 0x9e3272c5 ^ (val << 20);
    ulong b = 0x6a22b5a1 ^ (val << 10);
    ulong c = 0x23a4511b ^ val;

    OptHashMasher(a, b, c);

    return ((size_t)c);
}


template <>
size_t
OptHash<int>::operator()(const int val) const
{
    ulong a = 0x9e3272c5 ^ (val << 8);
    ulong b = 0x6a22b5a1 ^ val;
    ulong c = 0x23a4511b ^ (val << 12);

    OptHashMasher(a, b, c);

    return ((size_t)c);
}


template <>
size_t
OptHash<uint>::operator()(const uint val) const
{
    ulong a = 0x9e3272c5 ^ (val << 8);
    ulong b = 0x6a22b5a1 ^ val;
    ulong c = 0x23a4511b ^ (val << 12);

    OptHashMasher(a, b, c);

    return ((size_t)c);
}


template <>
size_t
OptHash<long>::operator()(const long val) const
{
    ulong a = 0x9e3272c5 ^ (val << 16);
    ulong b = 0x6a22b5a1 ^ (val >> 16);
    ulong c = 0x23a4511b ^ val;

    OptHashMasher(a, b, c);

    return ((size_t)c);
}


template <>
size_t
OptHash<ulong>::operator()(const ulong val) const
{
    ulong a = 0x9e3272c5 ^ (val << 16);
    ulong b = 0x6a22b5a1 ^ (val >> 16);
    ulong c = 0x23a4511b ^ val;

    OptHashMasher(a, b, c);

    return ((size_t)c);
}


template <>
size_t
OptHash<longlong>::operator()(const longlong val) const
{
    ulong a = 0x9e3272c5 ^ (val >> 32);
    ulong b = 0x6a22b5a1 ^ (val >> 16);
    ulong c = 0x23a4511b ^ (ulong)val;

    OptHashMasher(a, b, c);

    return ((size_t)c);
}


template <>
size_t
OptHash<ulonglong>::operator()(const ulonglong val) const
{
    ulong a = 0x9e3272c5 ^ (val >> 32);
    ulong b = 0x6a22b5a1 ^ (val >> 16);
    ulong c = 0x23a4511b ^ (ulong)val;

    OptHashMasher(a, b, c);

    return ((size_t)c);
}
