//
//  processContours.cpp
//  font-rendering
//
//  Created by Taanish Reja on 8/18/25.
//

#include "processContours.hpp"



simd_float2 getMidpoint(simd_float2 pointA, simd_float2 pointB) {
    return simd_float2{(pointA.x+pointB.x)/2,(pointA.y+pointB.y)/2};
}


Contour processContour(FT_Vector* rawPoints, unsigned char* tags, int start, int end, int offset) {
    Quad quad {.topLeft = {0,0}, .bottomRight = {0,0}};
    std::vector<int> offsets {};
    std::vector<simd_float2> points {};
    std::vector<simd_float2> pointsBuffer {};

    Segment currentSegmentType = Segment::Line;
    
    for (int p = start; p <= end; ++p) {
        simd_float2 currentPoint {(float)rawPoints[p].x, (float)rawPoints[p].y};
        
        // quad handling
        if (currentPoint.x < quad.topLeft.x)
            quad.topLeft.x = currentPoint.x;
        
        if (currentPoint.y < quad.topLeft.y)
            quad.topLeft.y = currentPoint.y - 1.0f;
        
        if (currentPoint.x > quad.bottomRight.x)
            quad.bottomRight.x = currentPoint.x;
        
        if (currentPoint.y > quad.bottomRight.y)
            quad.bottomRight.y = currentPoint.y + 1.0f;

        // contour handling
        switch (tags[p]) {
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
                // handle in future
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
                        auto midPoint = getMidpoint(lastPoint, currentPoint);
                        pointsBuffer.push_back(midPoint);
                        pointsBuffer.push_back(currentPoint);
                        points.insert(points.end(), pointsBuffer.begin(), pointsBuffer.end());
                        pointsBuffer.clear();
                    }else {
                        offsets.push_back(offset+p);
                    }
                    
                    pointsBuffer.push_back(currentPoint);
                }
        }
    }

    
    if (pointsBuffer.size() == 2) {
        pointsBuffer.push_back(points[0]);
        points.insert(points.end(), pointsBuffer.begin(), pointsBuffer.end());
        pointsBuffer.clear();
    }else if (pointsBuffer.size() == 1) {
        auto firstPoint = points[0];
        auto lastPoint = pointsBuffer.back();
        auto midPoint = getMidpoint(lastPoint, firstPoint);
        pointsBuffer.push_back(midPoint);
        pointsBuffer.push_back(firstPoint);
        points.insert(points.end(), pointsBuffer.begin(), pointsBuffer.end());
        pointsBuffer.clear();
    }

    return {.quad = quad, .points = points, .offsets = offsets};
}

Glyph processContours(FT_Outline* outlinePtr, int offset)
{
    // FT metadata
    FT_Outline outline = *outlinePtr;
    
    int numContours = outline.n_contours;
    unsigned short* contours = outline.contours;

    // raw points and their tags
    FT_Vector* rawPoints = outline.points;
    unsigned char* tags = outline.tags;

    // iterate over contours
    Quad quad {.topLeft = {0,0}, .bottomRight = {0,0}};
    std::vector<simd_float2> points;
    
    int contourStart = 0;
    std::vector<int> contourOffsets;
    std::vector<int> contourSizes;
    
    for (int c = 0; c < numContours; ++c) {
        Contour contour = processContour(rawPoints, tags, contourStart, contours[c], offset);
        
        // handle quad
        if (contour.quad.topLeft.x < quad.topLeft.x)
            quad.topLeft.x = contour.quad.topLeft.x;
        
        if (contour.quad.topLeft.y < quad.topLeft.y)
            quad.topLeft.y = contour.quad.topLeft.y;
        
        if (contour.quad.bottomRight.x > quad.bottomRight.x)
            quad.bottomRight.x = contour.quad.bottomRight.x;
        
        if (contour.quad.bottomRight.y > quad.bottomRight.y)
            quad.bottomRight.y = contour.quad.bottomRight.y;
        
        
        // flatten points
        points.insert(points.end(), contour.points.begin(), contour.points.end());

        // process offsets
        contourOffsets.push_back(offset + contourStart);
        
        // process sizes
        contourSizes.push_back(contour.points.size());
        
        contourStart = contours[c] + 1;
    }
    

    return {.quad = quad, .points = points, .numContours = numContours, .contourOffsets = contourOffsets, .contourSizes = contourSizes};
}

