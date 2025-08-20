//
//  processContours.hpp
//  font-rendering
//
//  Created by Taanish Reja on 8/18/25.
//
#ifndef PROCESS_CONTOURS_HPP
#define PROCESS_CONTOURS_HPP
#include <iostream>
#include <iomanip>
#include <cmath>
#include "freetype.hpp"
#include "metal_imports.hpp"
#include "bezier.hpp"
#include "windowConstants.hpp"

//// vertex points are just the 1st buffer
//// std::vector<GlyphMeta> all the metadata, can just be ints
//
//// Fragment Shader
//// i dont need to differentiate between quadratic and cubic just yet i dont think
//// std::vector<simd_float2> bezier points; buffer 1
//
//struct GlyphMeta {
//    int bezierIndex;
//    int numContours;
//    int contourPoints;
//};
//
//// std::vector<int>; buffer 2
//
//struct Uniforms {
//    simd_float3 color;
//}; // should not be embedded in glyph meta; does not vary; buffer 3
//
//
//// std::vector<Uniforms>; buffer 3

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

#endif
