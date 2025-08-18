//
//  renderFace.hpp
//  font-rendeirng
//
//  Created by Taanish Reja on 7/25/25.
//

#ifndef RENDER_FACE_H
#define RENDER_FACE_H
#include <iostream>
#include <iomanip>
#include <cmath>
#include "freetype.hpp"
#include "metal_imports.hpp"
#include "bezier.hpp"
#include "windowConstants.hpp"


enum class Segment {
    Line,
    Conic,
    Cubic
};

simd_float2 toNDC(const simd_float2& pt);

simd_float2 getMidpoint(simd_float2 pointA, simd_float2 pointB);
void drawBezier(const std::vector<simd_float2>& controlPoints, std::vector<simd_float2>& curve);
std::vector<simd_float2> drawContour(FT_Vector* points,
                                    unsigned char* tags,
                                    long start,
                                    long end,
                                    short unitsPerEm,
                                    float penX = 0.0,
                                    float penY = 0.0);

//std::vector<std::vector<simd_float2>> drawContours(char ch,
//                                                FT_Face face,
//                                                float penX = 0.0,
//                                                float penY = 0.0);
std::tuple<std::vector<std::vector<simd_float2>>, simd_float2, simd_float2>
drawContours(FT_Outline* outlinePtr, float penX, float penY);
#endif
