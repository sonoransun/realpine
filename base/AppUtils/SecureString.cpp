/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <SecureString.h>
#include <Platform.h>
#include <cstring>
#include <algorithm>

#ifdef ALPINE_PLATFORM_POSIX
#include <strings.h>
#endif


SecureString::SecureString (const string & s)
    : data_(s)
{
}


SecureString::SecureString (string && s)
    : data_(std::move(s))
{
}


SecureString::~SecureString ()
{
    secureErase();
}


SecureString::SecureString (SecureString && other) noexcept
    : data_(std::move(other.data_))
{
    other.secureErase();
}


SecureString &
SecureString::operator= (SecureString && other) noexcept
{
    if (this != &other) {
        secureErase();
        data_ = std::move(other.data_);
        other.secureErase();
    }
    return *this;
}


void
SecureString::assign (const string & s)
{
    secureErase();
    data_ = s;
}


void
SecureString::assign (string && s)
{
    secureErase();
    data_ = std::move(s);
}


void
SecureString::clear ()
{
    secureErase();
}


const string &
SecureString::value () const
{
    return data_;
}


bool
SecureString::empty () const
{
    return data_.empty();
}


bool
SecureString::equals (const string & other) const
{
    // Constant-time comparison: always iterate the longer length to avoid
    // leaking length information through timing side channels.
    const auto & a = data_;
    const auto & b = other;
    size_t maxLen = std::max(a.size(), b.size());

    volatile unsigned char accumulator = 0;

    // XOR corresponding bytes; out-of-range indices contribute 0xFF
    // to ensure length mismatches are detected without short-circuiting.
    for (size_t i = 0; i < maxLen; ++i) {
        unsigned char byteA = (i < a.size()) ? static_cast<unsigned char>(a[i]) : 0xFF;
        unsigned char byteB = (i < b.size()) ? static_cast<unsigned char>(b[i]) : 0xFF;
        accumulator |= (byteA ^ byteB);
    }

    // Also flag a difference if the lengths don't match
    accumulator |= static_cast<unsigned char>(a.size() != b.size());

    return accumulator == 0;
}


void
SecureString::secureErase ()
{
    if (!data_.empty()) {
#ifdef ALPINE_PLATFORM_WINDOWS
        SecureZeroMemory(data_.data(), data_.size());
#elif defined(ALPINE_PLATFORM_POSIX)
        // bzero is available on all POSIX platforms; volatile prevents optimization
        volatile void *p = data_.data();
        memset(const_cast<void *>(p), 0, data_.size());
#endif
        data_.clear();
    }
}
