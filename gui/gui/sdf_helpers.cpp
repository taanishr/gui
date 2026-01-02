//
//  sdf_helpers.cpp
//  gui
//
//  Created by Taanish Reja on 8/24/25.
//

#include "sdf_helpers.hpp"

float rounded_rect_sdf(simd_float2 pt, simd_float2 halfExtent, float r)
{
    r = simd_clamp(r, 0.0f, fmin(halfExtent.x, halfExtent.y));
    simd_float2 q = simd_abs(pt) - halfExtent + r;
    return simd_length(simd_max(q, 0.0f)) + simd_min(simd_max(q.x,q.y),0.0f) - r;
}

