//
//  renderFace.cpp
//  font-rendeirng
//
//  Created by Taanish Reja on 7/25/25.
//

#include "renderFace.hpp"

simd_float2 getMidpoint(simd_float2 pointA, simd_float2 pointB) {
    return simd_float2{(pointA[0]+pointB[0])/2,(pointA[1]+pointB[1])/2};
}

bool isFlat(const std::vector<simd_float2>& controlPoints, float threshold = 16)
{
    float distanceSum = 0.0;
    simd_float2 firstPoint = controlPoints.front();
    simd_float2 lastPoint = controlPoints.back();

    float thresholdSq = threshold * threshold;
    float dx = firstPoint[0]-lastPoint[0];
    float dy = firstPoint[1]-lastPoint[1];
    float denomSq = dx*dx+dy*dy;

    for (int i = 1; i < controlPoints.size() - 1; ++i) {
        simd_float2 currPoint = controlPoints[i];
        float num = (dy*currPoint[0] - dx*currPoint[1] + dx*firstPoint[1] - dy*firstPoint[0]);
        distanceSum += (num*num)/denomSq;
        if (distanceSum >= thresholdSq) return false;
    }

    return true;
}

void drawBezier(const std::vector<simd_float2>& controlPoints, std::vector<simd_float2>& curve) {
    curve.push_back(controlPoints[0]);
    if (isFlat(controlPoints)) {
        curve.push_back(controlPoints.back());
        return;
    }

    auto [leftCurve, rightCurve] = splitBezier(controlPoints);

    drawBezier(leftCurve, curve);
    drawBezier(rightCurve, curve);
}



std::vector<simd_float2> drawContour(FT_Vector* points, unsigned char* tags, long start,
                                     long end, float penX) {
    std::vector<simd_float2> renderedPoints {};
    std::vector<simd_float2> renderedPointsBuffer {};

    Segment currentSegmentType = Segment::Line;
    
    for (long p = start; p <= end; ++p) {
        simd_float2 currentPoint {(float)points[p].x + penX, (float)points[p].y};

        switch (tags[p]) {
            case FT_CURVE_TAG_CONIC:
                if (currentSegmentType == Segment::Conic) {
                    simd_float2 prevPoint = renderedPointsBuffer.back();
                    simd_float2 midpoint = getMidpoint(prevPoint, currentPoint);
                    renderedPointsBuffer.push_back(midpoint);
                    std::vector<simd_float2> curve {};
                    drawBezier(renderedPointsBuffer, curve);
                    renderedPoints.insert(renderedPoints.end(), curve.begin(), curve.end());
                    renderedPointsBuffer.clear();
                    renderedPointsBuffer.push_back(midpoint);
                }
                renderedPointsBuffer.push_back(currentPoint);
                currentSegmentType = Segment::Conic;
                break;
            case FT_CURVE_TAG_CUBIC:
                renderedPointsBuffer.push_back(currentPoint);
                currentSegmentType = Segment::Cubic;
                break;
            default:
                if (currentSegmentType != Segment::Line) {
                    renderedPointsBuffer.push_back(currentPoint);
                    std::vector<simd_float2> curve {};
                    drawBezier(renderedPointsBuffer, curve);
                    renderedPoints.insert(renderedPoints.end(), curve.begin(), curve.end());
                    renderedPointsBuffer.clear();
                    renderedPointsBuffer.push_back(currentPoint);
                    currentSegmentType = Segment::Line;
                }else {
                    if (renderedPointsBuffer.size() == 1) {
                        renderedPoints.push_back(renderedPointsBuffer.back());
                        renderedPointsBuffer.clear();
                    }
                    renderedPointsBuffer.push_back(currentPoint);
                }
        }
    }
    
    if (renderedPointsBuffer.size() > 0) {
        renderedPointsBuffer.push_back(renderedPoints[0]);
        if (currentSegmentType != Segment::Line) {
            std::vector<simd_float2> curve {};
            drawBezier(renderedPointsBuffer, curve);
            renderedPoints.insert(renderedPoints.end(), curve.begin(), curve.end());
        }else {
            renderedPoints.insert(renderedPoints.end(), renderedPointsBuffer.begin(), renderedPointsBuffer.end());
        }
        renderedPointsBuffer.clear();
    }

    return renderedPoints;
}

std::vector<std::vector<simd_float2>> drawContours(char ch, FT_Face face, std::string_view fontPath, float penX)
{
    FT_Outline outline = face->glyph->outline;

    long numContours = outline.n_contours;
    unsigned short* contours = outline.contours;

    // points and their tags
    FT_Vector* points = outline.points;
    unsigned char* tags = outline.tags;

    // iterate over contours
    std::vector<std::vector<simd_float2>> renderedContours {};
    
    int contourStart = 0;
    for (int i = 0; i < numContours; ++i) {
        std::vector<simd_float2> contourPoints = drawContour(points, tags, contourStart, contours[i], penX);
        renderedContours.push_back(contourPoints);
        contourStart = contours[i]+1;
    }
    
    return renderedContours;
}
