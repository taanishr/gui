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
    float scale;
};

inline float2 toNDC(const float2 pt, float width = 512.0f, float height = 512.0f) {
    float ndcX = (pt.x / width) * 2.0f - 1.0f;
    float ndcY = 1.0f - (pt.y / height) * 2.0f;
    
    return {ndcX, ndcY};
}

inline float rounded_rect_sdf(float2 pt, float2 halfExtent, float2 r) {
    const float epsilon = 0.0001;

    r.x = clamp(r.x, epsilon, halfExtent.x);
    r.y = clamp(r.y, epsilon, halfExtent.y);

    float2 q = abs(pt) - halfExtent + r;

    float2 qNormalized = max(q, 0.0) / r;
    

    float distOutside = (length(qNormalized) - 1.0) *  min(r.x, r.y);
    float distInside = min(max(q.x,q.y), 0.0);

    return distOutside + distInside;
}