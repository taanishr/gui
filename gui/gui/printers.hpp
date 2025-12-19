//
//  printers.hpp
//  gui
//
//  Created by Taanish Reja on 8/20/25.
//

#pragma once

#include "metal_imports.hpp"
#include <iostream>
#include <print>

void printPoint(const simd_float2& pt, char delim = ' ');

template <>
struct std::formatter<simd_float2> : std::formatter<float>
{
    auto format(const simd_float2& v, format_context& ctx) const
    {
        return std::format_to(ctx.out(), "({}, {})", v.x, v.y);
    }
};

template <typename T>
void debug_print_vec(const std::vector<T>& v, std::string_view name = {})
{
    if (!name.empty())
        std::print("{} = ", name);

    std::print("[");
    for (size_t i = 0; i < v.size(); ++i) {
        if (i) std::print(", ");
        std::print("{}", v[i]);
    }
    std::println("]");
}

template <typename T, std::size_t N>
void debug_print_arr(const std::array<T, N>& a, std::string_view name = {})
{
    if (!name.empty())
        std::print("{} = ", name);

    std::print("[");
    for (std::size_t i = 0; i < N; ++i) {
        if (i) std::print(", ");
        std::print("{}", a[i]);
    }
    std::println("]");
}

template <typename T, std::size_t N>
void debug_print_arr(const T (&a)[N], std::string_view name = {})
{
    if (!name.empty())
        std::print("{} = ", name);

    std::print("[");
    for (std::size_t i = 0; i < N; ++i) {
        if (i) std::print(", ");
        std::print("{}", a[i]);
    }
    std::println("]");
}

template <typename T>
inline void debug_print_ptr(const T* data, std::size_t n, std::string_view name = {})
{
    if (!name.empty())
        std::print("{} = ", name);

    std::print("[");
    for (std::size_t i = 0; i < n; ++i) {
        if (i) std::print(", ");
        std::print("{}", data[i]);
    }
    std::println("]");
}
