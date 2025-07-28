//
//  bezier.cpp
//  font-rendeirng
//
//  Created by Taanish Reja on 7/24/25.
//

#include "bezier.hpp"


simd_float2 linearInterpolation(simd_float2 c1, simd_float2 c2, float t) {
    return {c1[0] + t*(c2[0]-c1[0]), c1[1] + t*(c2[1]-c1[1])};
}

simd_float2 quadraticInterpolation(simd_float2 c1, simd_float2 c2, simd_float2 c3, float t)
{
    simd_float2 p1 = linearInterpolation(c1, c2, t);
    simd_float2 p2 = linearInterpolation(c2, c3, t);

    return linearInterpolation(p1, p2, t);
}


simd_float2 cubicInterpolation(simd_float2 c1, simd_float2 c2, simd_float2 c3, simd_float2 c4, float t)
{
    simd_float2 p11 = linearInterpolation(c1, c2, t);
    simd_float2 p12 = linearInterpolation(c2, c3, t);
    simd_float2 p13 = linearInterpolation(c3, c4, t);

    simd_float2 p21 = linearInterpolation(p11, p12, t);
    simd_float2 p22 = linearInterpolation(p12, p13, t);

    return linearInterpolation(p21, p22, t);
}
