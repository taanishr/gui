//
//  bezier.cpp
//  font-rendeirng
//
//  Created by Taanish Reja on 7/24/25.
//

#include "bezier.hpp"

simd_float2 quadraticInterpolation(simd_float2 c1, simd_float2 c2, simd_float2 c3, float t, float resolution)
{
    float t1dx = (c2[0]-c1[0])/resolution;
    float t1dy = (c2[1]-c1[1])/resolution;
    float t2dx = (c3[0]-c2[0])/resolution;
    float t2dy = (c3[1]-c2[1])/resolution;


    float c1x = c1[0] + t1dx*t;
    float c1y = c1[1] + t1dy*t;
    float c2x = c2[0] + t2dx*t;
    float c2y = c2[1] + t2dy*t;

    float cdx = (c2x-c1x)/resolution;
    float cdy = (c2y-c1y)/resolution;

    return simd_float2{c1x+cdx*t,c1y+cdy*t};
}

simd_float2 cubicInterpolation(simd_float2 c1, simd_float2 c2, simd_float2 c3, simd_float2 c4, float t, float resolution)
{
    float cmdx = (c3[0]-c2[0])/resolution;
    float cmdy = (c3[1]-c2[1])/resolution;

    simd_float2 cm {c2[0] + cmdx*t, c2[1] + cmdy*t};

    float t1dx = (cm[0]-c1[0])/resolution;
    float t1dy = (cm[1]-c1[1])/resolution;
    float t2dx = (c4[0]-cm[0])/resolution;
    float t2dy = (c4[1]-cm[1])/resolution;

    float c1x = c1[0] + t1dx*t;
    float c1y = c1[1] + t1dy*t;
    float c2x = cm[0] + t2dx*t;
    float c2y = cm[1] + t2dy*t;

    float cdx = (c2x-c1x)/resolution;
    float cdy = (c2y-c1y)/resolution;

    return simd_float2{c1x+cdx*t,c1y+cdy*t};
}
