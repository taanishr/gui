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
};

struct Glyph {
    Quad quad;
    std::vector<simd_float2> points;
    int numContours;
    std::vector<int> contourSizes;
    FT_Glyph_Metrics metrics;
};

Contour processContour(FT_Vector* rawPoints, unsigned char* tags, int start, int end, float ascender);
Glyph processContours(FT_Face glyphMeta);
