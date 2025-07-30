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
//std::vector<simd_float2> drawBezier(Segment segment, std::vector<simd_float2> buffer, float resolution);
void drawBezier(const std::vector<simd_float2>& controlPoints, std::vector<simd_float2>& curve);
std::vector<simd_float2> drawContour(FT_Vector* points,
                                    unsigned char* tags,
                                    long start,
                                    long end,
                                    float resolution,
                                    float offsetX = 0.0,
                                    float offsetY = 0.0);
std::vector<std::vector<simd_float2>> drawContours(char ch,
                                                FT_Library ft,
                                                FT_Face face,
                                                std::string_view fontPath,
                                                float resolution,
                                                float offsetX = 0.0,
                                                float offsetY = 0.0);
