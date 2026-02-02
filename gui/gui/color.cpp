//
//  color.cpp
//  gui
//
//  Created by Taanish Reja on 8/29/25.
//
#include "color.hpp"

RGB::RGB(int r, int g, int b, float a)
{
    r = std::clamp(r, 0, 255);
    g = std::clamp(g, 0, 255);
    b = std::clamp(b, 0, 255);
    normalized = simd_float4{r/255.0f, g/255.0f, b/255.0f, a};
}
    
simd_float4 RGB::get() const
{
    return normalized;
}

/*
inline float rounded_rect_sdf(float2 pt, float2 halfExtent, float r)
{
    r = clamp(r, 0.0, min(halfExtent.x, halfExtent.y));
    float2 q = abs(pt) - halfExtent + r;
    return length(max(q, 0.0)) + min(max(q.x,q.y),0.0) - r;
}
*/

Hex::Hex(int hexCode)
{
    int r = std::clamp((hexCode >> 16) & 0xff, 0, 255);
    int g = std::clamp((hexCode >> 8) & 0xff, 0, 255);
    int b = std::clamp((hexCode) & 0xff, 0, 255);
    
    normalized = simd_float4{r/255.0f, g/255.0f, b/255.0f, 1.0};
}

simd_float4 Hex::get() const
{
    return normalized;
}
