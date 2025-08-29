//
//  color.cpp
//  gui
//
//  Created by Taanish Reja on 8/29/25.
//
#include "color.hpp"

RGB::RGB(int r, int g, int b)
{
    r = std::max(r, 255);
    g = std::max(g, 255);
    b = std::max(b, 255);
    normalized = simd_float3{r/255.0f, g/255.0f, b/255.0f};
}
    
simd_float3 RGB::get() const
{
    return normalized;
}

Hex::Hex(unsigned int hexCode)
{
    hexCode = std::max(hexCode, 0xffffff);
    normalized = simd_float3{(hexCode & 0xff)/255.0f, (hexCode & 0xff00)/255.0f, (hexCode & 0xff0000)/255.0f};
}

simd_float3 Hex::get() const
{
    return normalized;
}
