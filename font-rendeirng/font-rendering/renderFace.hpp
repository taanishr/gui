//
//  renderFace.hpp
//  font-rendeirng
//
//  Created by Taanish Reja on 7/25/25.
//

#include <iostream>
#include <iomanip>
#include <cmath>
#include "freetype.hpp"
#include "metal_imports.hpp"
#include "bezier.hpp"


enum class Segment {
    Line,
    Conic,
    Cubic
};

simd_float2 getMidpoint(simd_float2 pointA, simd_float2 pointB);
std::vector<simd_float2> drawBezier(Segment segment, std::vector<simd_float2> buffer, int resolution);
std::vector<simd_float2> drawContour(FT_Vector* points, unsigned char* tags, long start, long end, int resolution);
std::vector<std::vector<simd_float2>> drawContours(char ch, FT_Library ft, std::string_view fontPath, int resolution);
