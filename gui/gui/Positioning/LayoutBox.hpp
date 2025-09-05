//
//  LayoutBox.hpp
//  gui
//
//  Created by Taanish Reja on 9/4/25.
//
#include "metal_imports.hpp"

enum class PositioningType {
    Absolute,
    Relative
};

struct LayoutBox {
    LayoutBox();
    
    PositioningType positioningType;
    
    float width, height;
    float minWidth, maxWidth;
    float minHeight, maxHeight;
    
    simd_float2 offset;
    simd_float2 globalPosition;
};
