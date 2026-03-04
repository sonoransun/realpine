/// Copyright (C) 2026 sonoransun — see LICENCE.txt

#pragma once
#include <expected>
#include <string_view>

enum class AlpineError {
    NotFound,
    InvalidArgument,
    NotInitialized,
    Timeout,
    ConnectionFailed,
    PermissionDenied,
    AlreadyExists,
    NotSupported,
    InternalError
};

constexpr std::string_view errorToString(AlpineError error) {
    switch (error) {
        case AlpineError::NotFound:          return "Not found";
        case AlpineError::InvalidArgument:   return "Invalid argument";
        case AlpineError::NotInitialized:    return "Not initialized";
        case AlpineError::Timeout:           return "Timeout";
        case AlpineError::ConnectionFailed:  return "Connection failed";
        case AlpineError::PermissionDenied:  return "Permission denied";
        case AlpineError::AlreadyExists:     return "Already exists";
        case AlpineError::NotSupported:      return "Not supported";
        case AlpineError::InternalError:     return "Internal error";
    }
    return "Unknown error";
}

template<typename T>
using Result = std::expected<T, AlpineError>;

using Status = std::expected<void, AlpineError>;
