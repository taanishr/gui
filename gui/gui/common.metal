//
//  to_ndc.metal
//  gui
//
//  Created by Taanish Reja on 8/20/25.
//

#pragma once

#include <metal_stdlib>
using namespace metal;

struct FrameInfo {
    float width;
    float height;
};

inline float2 toNDC(const float2 pt, float width = 512.0f, float height = 512.0f) {
    float ndcX = (pt.x / width) * 2.0f - 1.0f;
    float ndcY = 1.0f - (pt.y / height) * 2.0f;
    
    return {ndcX, ndcY};
}

inline float rounded_rect_sdf(float2 pt, float2 halfExtent, float r)
{
    r = clamp(r, 0.0, min(halfExtent.x, halfExtent.y));
    float2 q = abs(pt) - halfExtent + r;
    return length(max(q, 0.0)) + min(max(q.x,q.y),0.0) - r;
}
