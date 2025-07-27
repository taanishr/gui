//
//  renderFace.cpp
//  font-rendeirng
//
//  Created by Taanish Reja on 7/25/25.
//

#include "renderFace.hpp"

//std::vector<simd_float2> renderPoints(char ch, FT_Library ft, std::string_view fontPath, int resolution) {
//    FT_Face face;
//
//    FT_New_Face(ft, fontPath.data(), 0, &face);
//
//    FT_Set_Pixel_Sizes(face, 0, 0.2);
//
//    FT_Load_Char(face, ch, FT_LOAD_RENDER);
//
//    FT_Outline g = face->glyph->outline;
//
//    FT_Vector* points = g.points;
//    unsigned char* tags = g.tags;
//
//    int currentPoint = 0;
//    int pointsLeft = g.n_points;
//
//    bool seenControlGroupStart = false;
//    std::vector<simd_float2> renderedPoints {};
//    std::vector<simd_float2> pointsBuffer {};
//
//    while (pointsLeft > 0) {
//        simd_float2 point = simd_float2{(float)points[currentPoint].x, (float)points[currentPoint].y};
//        
//        if (FT_CURVE_TAG(tags[currentPoint]) == FT_CURVE_TAG_ON) {
//            if (seenControlGroupStart) {
//                if (pointsBuffer.size() == 3) {
//                    for (int t = 0; t <= resolution; ++t)
//                        renderedPoints.push_back(quadraticInterpolation(pointsBuffer[0], pointsBuffer[1], pointsBuffer[2], t, resolution));
//                    pointsBuffer.clear();
//                    seenControlGroupStart = false;
//                }else if (pointsBuffer.size() == 4) {
//                    for (int t = 0; t <= resolution; ++t)
//                        renderedPoints.push_back(cubicInterpolation(pointsBuffer[0], pointsBuffer[1], pointsBuffer[2], pointsBuffer[3], t, resolution));
//                    pointsBuffer.clear();
//                    seenControlGroupStart = false;
//                }else {
//                    renderedPoints.insert(renderedPoints.end(), pointsBuffer.begin(), pointsBuffer.end());
//                    pointsBuffer.clear();
//                }
//                pointsBuffer.push_back(point);
//            }else {
//                seenControlGroupStart = true;
//                pointsBuffer.push_back(point);
//            }
//        }else {
//            pointsBuffer.push_back(point);
//        }
//
//        --pointsLeft;
//        ++currentPoint;
//    }
//
//    FT_Done_Face(face);
//
//    return renderedPoints;
//}

simd_float2 getMidpoint(simd_float2 pointA, simd_float2 pointB) {
    return simd_float2{(pointA[0]+pointB[0])/2,(pointA[1]+pointB[1])/2};
}

std::vector<simd_float2> drawBezier(Segment segment, std::vector<simd_float2> buffer, int resolution) {
    std::vector<simd_float2> curve;
    if (segment == Segment::Conic) {
        for (int t = 0; t <= resolution; ++t) {
            curve.push_back(quadraticInterpolation(buffer[0], buffer[1], buffer[2], t, resolution));
        }
    }else {
        for (int t = 0; t <= resolution; ++t) {
            curve.push_back(cubicInterpolation(buffer[0], buffer[1], buffer[2], buffer[3], t, resolution));
        }
    }
    return curve;
}


std::vector<simd_float2> drawContour(FT_Vector* points, unsigned char* tags, long start, long end, int resolution) {
    std::vector<simd_float2> renderedPoints {};
    std::vector<simd_float2> renderedPointsBuffer {};

    Segment currentSegmentType = Segment::Line;
    
    for (long p = start; p <= end; ++p) {
        simd_float2 currentPoint {(float)points[p].x / 100, (float)points[p].y / 100};

        switch (tags[p]) {
            case FT_CURVE_TAG_CONIC:
                if (currentSegmentType == Segment::Conic) {
                    simd_float2 prevPoint = renderedPointsBuffer.back();
                    simd_float2 midpoint = getMidpoint(prevPoint, currentPoint);
                    renderedPointsBuffer.push_back(midpoint);
                    std::vector<simd_float2> curve = drawBezier(currentSegmentType, renderedPointsBuffer, resolution);
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
                    std::vector<simd_float2> curve = drawBezier(currentSegmentType, renderedPointsBuffer, resolution);
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

    return renderedPoints;
}

std::vector<std::vector<simd_float2>> drawContours(char ch, FT_Library ft, std::string_view fontPath, int resolution)
{
    FT_Face face;

    FT_New_Face(ft, fontPath.data(), 0, &face);

    FT_Set_Pixel_Sizes(face, 0, 1);

    FT_Load_Char(face, ch, FT_LOAD_RENDER);

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
        std::vector<simd_float2> contourPoints = drawContour(points, tags, contourStart, contours[i], resolution);
        renderedContours.push_back(contourPoints);
        contourStart = contours[i]+1;
    }

    return renderedContours;
}
