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

struct RGB {
    RGB(int r, int g, int b);
    
    simd_float3 get() const;
    
    simd_float3 normalized;
};

struct Hex {
    Hex(unsigned int hexCode);
    
    simd_float3 get() const;

    unsigned int hex;
    
    simd_float3 normalized;
};

using Color = std::variant<RGB, Hex>;
