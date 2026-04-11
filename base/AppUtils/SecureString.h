/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>


class SecureString
{
  public:
    SecureString() = default;

    explicit SecureString(const string & s);

    explicit SecureString(string && s);

    ~SecureString();


    // Non-copyable
    SecureString(const SecureString &) = delete;
    SecureString & operator=(const SecureString &) = delete;

    // Movable — securely erases the source after move
    SecureString(SecureString && other) noexcept;
    SecureString & operator=(SecureString && other) noexcept;


    void assign(const string & s);

    void assign(string && s);

    void clear();

    [[nodiscard]] const string & value() const;

    [[nodiscard]] bool empty() const;

    [[nodiscard]] bool equals(const string & other) const;


  private:
    string data_;

    void secureErase();
};
