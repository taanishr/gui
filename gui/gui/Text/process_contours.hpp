//
//  processContours.hpp
//  font-rendering
//
//  Created by Taanish Reja on 8/18/25.
//
#pragma once

#include <iostream>
#include <iomanip>
#include <cmath>
#include "freetype.hpp"
#include "printers.hpp"
#include "metal_imports.hpp"

enum class Segment {
    Line,
    Conic,
    Cubic
};

struct Quad {
    simd_float2 topLeft;
    simd_float2 bottomRight;
};

struct Contour {
    Quad quad;
    std::vector<simd_float2> points;
    std::vector<int> offsets;
};

struct Glyph {
    Quad quad;
    std::vector<simd_float2> points;
    int numContours;
    std::vector<int> contourSizes;
    std::vector<int> contourOffsets;
};

Contour processContour(FT_Vector* rawPoints, unsigned char* tags, int start, int end, int offset = 0);
Glyph processContours(FT_Outline* outlinePtr, int offset);
