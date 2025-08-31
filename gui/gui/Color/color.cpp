//
//  color.cpp
//  gui
//
//  Created by Taanish Reja on 8/29/25.
//
#include "color.hpp"

RGB::RGB(int r, int g, int b)
{
    r = simd::clamp(r, 0, 255);
    g = simd::clamp(g, 0, 255);
    b = simd::clamp(b, 0, 255);
    normalized = simd_float3{r/255.0f, g/255.0f, b/255.0f};
}
    
simd_float3 RGB::get() const
{
    return normalized;
}

Hex::Hex(int hexCode)
{
    int r = simd::clamp((hexCode >> 16) & 0xff, 0, 255);
    int g = simd::clamp((hexCode >> 8) & 0xff, 0, 255);
    int b = simd::clamp((hexCode) & 0xff, 0, 255);
    
    normalized = simd_float3{r/255.0f, g/255.0f, b/255.0f};
}

simd_float3 Hex::get() const
{
    return normalized;
}
