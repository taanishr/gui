//
//  color.hpp
//  gui
//
//  Created by Taanish Reja on 8/28/25.
//

#pragma once
#include <variant>
#include "metal_imports.hpp"
#include <string>
#include <concepts>
#include <algorithm>

struct RGB {
    RGB(int r, int g, int b, float a);
    
    simd_float4 get() const;
    
    simd_float4 normalized;
};

struct Hex {
    Hex(int hexCode);
    
    simd_float4 get() const;
    
    simd_float4 normalized;
};

template <typename T>
concept ColorType = requires(T t) {
    { t.get() } -> std::convertible_to<simd_float4>;
};
