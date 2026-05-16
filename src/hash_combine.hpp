//
//  hash_combine.hpp
//  gui
//
//  Created by Taanish Reja on 9/11/25.
//

#pragma once

#include <functional>

template <typename T>
inline void hash_combine(std::size_t& s, const T& v)
{
    std::hash<T> h;
    s ^= h(v) + 0x9e3779b9 + (s << 6) + (s >> 2);
}
