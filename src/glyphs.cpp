//
//  processContours.cpp
//  font-rendering
//
//  Created by Taanish Reja on 8/18/25.
//

#include "glyphs.hpp"
#include "freetype/ftglyph.h"
#include <limits>
#include "freetype/ftbbox.h"


simd_float2 getMidpoint(simd_float2 pointA, simd_float2 pointB) {
    return simd_float2{(pointA.x+pointB.x)/2,(pointA.y+pointB.y)/2};
}

simd_float2 getPoint(FT_Vector point, float ascender) {
    return {
        static_cast<float>(point.x),
        -static_cast<float>(point.y) + ascender
    };
}


Contour processContour(
    FT_Vector* rawPoints,
    unsigned char* tags,
    int start,
    int end,
    float ascender,
    CurveType curveType
) {
    Quad quad {.topLeft = {std::numeric_limits<float>::max(), std::numeric_limits<float>::max()}, .bottomRight = {std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest()}};

    std::vector<simd_float2> points {};
    std::vector<simd_float2> pointsBuffer {};

    int firstTag = FT_CURVE_TAG(tags[start]);
    int lastTag = FT_CURVE_TAG(tags[end]);
    int firstPointToProcess = start;
    int lastPointToProcess = end;
    simd_float2 contourStartPoint;

    if (firstTag == FT_CURVE_TAG_CONIC) {
        auto firstPoint = getPoint(rawPoints[start], ascender);
        auto lastPoint = getPoint(rawPoints[end], ascender);
        if (lastTag == FT_CURVE_TAG_ON) {
            contourStartPoint = lastPoint;
            lastPointToProcess = end - 1;
        } else {
            contourStartPoint = getMidpoint(firstPoint, lastPoint);
        }
    } else {
        contourStartPoint = getPoint(rawPoints[start], ascender);
        firstPointToProcess = start + 1;
    }

    pointsBuffer.push_back(contourStartPoint);

    Segment currentSegmentType = Segment::Line;

    for (int p = start; p <= end; ++p) {
        auto currentPoint = getPoint(rawPoints[p], ascender);

        // quad handling
        if (currentPoint.x < quad.topLeft.x)
            quad.topLeft.x = currentPoint.x;
        
        if (currentPoint.y < quad.topLeft.y)
            quad.topLeft.y = currentPoint.y;
        
        if (currentPoint.x > quad.bottomRight.x)
            quad.bottomRight.x = currentPoint.x;
        
        if (currentPoint.y > quad.bottomRight.y)
            quad.bottomRight.y = currentPoint.y;

    }

    for (int p = firstPointToProcess; p <= lastPointToProcess; ++p) {
        auto currentPoint = getPoint(rawPoints[p], ascender);

        // contour handling
        switch (FT_CURVE_TAG(tags[p])) {
            case FT_CURVE_TAG_CONIC:
                if (currentSegmentType == Segment::Conic) {
                    auto prevPoint = pointsBuffer.back();
                    auto midpoint = getMidpoint(prevPoint, currentPoint);
                    pointsBuffer.push_back(midpoint);
                    points.insert(points.end(), pointsBuffer.begin(), pointsBuffer.end());
                    pointsBuffer.clear();
                    pointsBuffer.push_back(midpoint);
                }
                
                pointsBuffer.push_back(currentPoint);
                currentSegmentType = Segment::Conic;
                break;
            case FT_CURVE_TAG_CUBIC: // for open type
                pointsBuffer.push_back(currentPoint);
                currentSegmentType = Segment::Cubic;
                break;
            default:
                if (currentSegmentType != Segment::Line) {
                    pointsBuffer.push_back(currentPoint);
                    points.insert(points.end(), pointsBuffer.begin(), pointsBuffer.end());
                    pointsBuffer.clear();
                    pointsBuffer.push_back(currentPoint);
                    currentSegmentType = Segment::Line;
                }else {
                    if (pointsBuffer.size() == 1) {
                        auto lastPoint = pointsBuffer.back();
                        if (curveType == CurveType::Cubic) {
                            auto delta = currentPoint - lastPoint;
                            pointsBuffer.push_back(lastPoint + delta / 3.0f);
                            pointsBuffer.push_back(lastPoint + 2.0f * delta / 3.0f);
                        } else {
                            pointsBuffer.push_back(getMidpoint(lastPoint, currentPoint));
                        }
                        pointsBuffer.push_back(currentPoint);
                        points.insert(points.end(), pointsBuffer.begin(), pointsBuffer.end());
                        pointsBuffer.clear();
                    }
                    
                    pointsBuffer.push_back(currentPoint);
                }
        }
    }

    
    switch (currentSegmentType) {
        case Segment::Conic:
            if (pointsBuffer.size() == 2) {
                pointsBuffer.push_back(contourStartPoint);
                points.insert(points.end(), pointsBuffer.begin(), pointsBuffer.end());
            }
            break;
        case Segment::Cubic:
            if (pointsBuffer.size() == 3) {
                pointsBuffer.push_back(contourStartPoint);
                points.insert(points.end(), pointsBuffer.begin(), pointsBuffer.end());
            }
            break;
        case Segment::Line:
            if (pointsBuffer.size() == 1) {
                auto lastPoint = pointsBuffer.back();
                if (curveType == CurveType::Cubic) {
                    auto delta = contourStartPoint - lastPoint;
                    pointsBuffer.push_back(lastPoint + delta / 3.0f);
                    pointsBuffer.push_back(lastPoint + 2.0f * delta / 3.0f);
                } else {
                    pointsBuffer.push_back(getMidpoint(lastPoint, contourStartPoint));
                }
                pointsBuffer.push_back(contourStartPoint);
                points.insert(points.end(), pointsBuffer.begin(), pointsBuffer.end());
            }
            break;
    }

    return {.quad = quad, .points = points};
}

Glyph processContours(FT_Face face)
{
    // FT metadata
    FT_Outline outline = face->glyph->outline;
    
    int numContours = outline.n_contours;
    unsigned short* contours = outline.contours;

    // raw points and their tags
    FT_Vector* rawPoints = outline.points;
    unsigned char* tags = outline.tags;

    // iterate over contours
    Quad quad {.topLeft = {0,0}, .bottomRight = {0,0}};
    std::vector<simd_float2> points;

    int contourStart = 0;
    std::vector<size_t> contourSizes;

    float ascender = face->size->metrics.ascender;

    CurveType curveType = CurveType::Quadratic;
    for (int p = 0; p < outline.n_points; ++p) {
        if (FT_CURVE_TAG(tags[p]) == FT_CURVE_TAG_CUBIC) {
            curveType = CurveType::Cubic;
            break;
        }
    }

    for (int c = 0; c < numContours; ++c) {
        Contour contour = processContour(
            rawPoints,
            tags,
            contourStart,
            contours[c],
            ascender,
            curveType
        );

        const size_t pointStride = curveType == CurveType::Cubic ? 4 : 3;
        if (contour.points.size() % pointStride != 0) {
            std::println(
                "invalid glyph contour: glyph={}, contour={}, curveType={}, points={}, stride={}",
                face->glyph->glyph_index,
                c,
                static_cast<uint32_t>(curveType),
                contour.points.size(),
                pointStride
            );
        }

        // handle quad
        if (contour.quad.topLeft.x < quad.topLeft.x)
            quad.topLeft.x = contour.quad.topLeft.x;

        if (contour.quad.topLeft.y < quad.topLeft.y)
            quad.topLeft.y = contour.quad.topLeft.y;

        if (contour.quad.bottomRight.x > quad.bottomRight.x)
            quad.bottomRight.x = contour.quad.bottomRight.x;

        if (contour.quad.bottomRight.y > quad.bottomRight.y)
            quad.bottomRight.y = contour.quad.bottomRight.y;


        quad.bottomRight += BASE_PIXEL_HEIGHT;

        // flatten points
        points.insert(points.end(), contour.points.begin(), contour.points.end());

        // process sizes
        contourSizes.push_back(contour.points.size());

        contourStart = contours[c] + 1;
    }

    return {
        .quad = quad,
        .curveType = curveType,
        .points = points,
        .numContours = numContours,
        .contourSizes = contourSizes
    };
}
