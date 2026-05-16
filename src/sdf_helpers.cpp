//
//  sdf_helpers.cpp
//  gui
//
//  Created by Taanish Reja on 8/24/25.
//

#include "sdf_helpers.hpp"
#include <simd/vector_types.h>

float rounded_rect_sdf(simd_float2 pt, simd_float2 halfExtent, simd_float2 r) {
    const float epsilon = 0.0001;

    r.x = simd::clamp(r.x, epsilon, halfExtent.x);
    r.y = simd::clamp(r.y, epsilon, halfExtent.y);

    simd_float2 q = simd::abs(pt) - halfExtent + r;
    
    simd_float2 qNormalized = simd_float2{
        std::max(q.x, 0.0f),
        std::max(q.y, 0.0f)
    } / r;
    

    float distOutside = (simd::length(qNormalized) - 1.0) *  std::min(r.x, r.y);
    float distInside = std::min(std::max(q.x,q.y), 0.0f);

    return distOutside + distInside;
}
