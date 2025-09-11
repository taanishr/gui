//
//  bounds.hpp
//  gui
//
//  Created by Taanish Reja on 9/6/25.
//

#pragma once

struct DrawableSize {
    float width;
    float height;
};

struct Bounds {
    simd_float2 topLeft;
    simd_float2 bottomRight;
};
