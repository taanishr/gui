//
//  bezier.cpp
//  font-rendeirng
//
//  Created by Taanish Reja on 7/24/25.
//

#include "bezier.hpp"


simd_float2 linearInterpolation(simd_float2 c1, simd_float2 c2, float t) {
    return {c1.x + t*(c2.x-c1.x), c1.y + t*(c2.y-c1.y)};
}

void splitBezierHelper(const std::vector<simd_float2>& controlPoints,
                                    std::vector<simd_float2>& leftCurve,
                                    std::vector<simd_float2>& rightCurve)
{
    leftCurve.push_back(controlPoints.front());
    rightCurve.insert(rightCurve.begin(), controlPoints.back());

    if (controlPoints.size() == 2) {
        auto lerp0 = linearInterpolation(controlPoints[0], controlPoints[1], 0.5);
        leftCurve.push_back(lerp0);
        rightCurve.insert(rightCurve.begin(), lerp0);
        return;
    }

    std::vector<simd_float2> itrpPoints {};
    for (int i = 0; i < controlPoints.size()-1; ++i)
        itrpPoints.push_back(linearInterpolation(controlPoints[i], controlPoints[i+1], 0.5));
    
    splitBezierHelper(itrpPoints, leftCurve, rightCurve);
}

std::tuple<std::vector<simd_float2>, std::vector<simd_float2>> splitBezier(const std::vector<simd_float2>& controlPoints)
{
    std::vector<simd_float2> leftCurve {};
    std::vector<simd_float2> rightCurve {};
    splitBezierHelper(controlPoints, leftCurve, rightCurve);
    return {leftCurve, rightCurve};
}
