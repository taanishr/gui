//
//  bezier.hpp
//  font-rendeirng
//
//  Created by Taanish Reja on 7/24/25.
//

#ifndef BEZIER_HPP
#define BEZIER_HPP
#include "metal_imports.hpp"

simd_float2 linearInterpolation(simd_float2 c1, simd_float2 c2, float t);

//simd_float2 quadraticInterpolation(simd_float2 c1, simd_float2 c2, simd_float2 c3, float t);
//
//simd_float2 cubicInterpolation(simd_float2 c1, simd_float2 c2, simd_float2 c3, simd_float2 c4, float t);

std::tuple<std::vector<simd_float2>, std::vector<simd_float2>> splitBezier(const std::vector<simd_float2>& controlPoints);

void splitBezierHelper(const std::vector<simd_float2>& controlPoints,
                                    std::vector<simd_float2>& leftCurve,
                       std::vector<simd_float2>& rightCurve);
#endif
