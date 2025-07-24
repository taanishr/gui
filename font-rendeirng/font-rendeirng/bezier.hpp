//
//  bezier.hpp
//  font-rendeirng
//
//  Created by Taanish Reja on 7/24/25.
//

#include "metal_imports.hpp"

simd_float2 quadraticInterpolation(simd_float2 c1, simd_float2 c2, simd_float2 c3, float t, float resolution = 100.0);

simd_float2 cubicInterpolation(simd_float2 c1, simd_float2 c2, simd_float2 c3, simd_float2 c4, float t, float resolution = 100.0);
