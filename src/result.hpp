#pragma once

#include <expected>
#include <system_error>

using Error = std::error_code;

template<typename T>
using Result = std::expected<T, Error>;
