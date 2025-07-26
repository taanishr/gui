//
//  renderFace.cpp
//  font-rendeirng
//
//  Created by Taanish Reja on 7/25/25.
//

#include "renderFace.hpp"

std::vector<simd_float2> renderPoints(char ch, FT_Library ft, std::string_view fontPath, int resolution) {
    FT_Face face;

    FT_New_Face(ft, fontPath.data(), 0, &face);

    FT_Set_Pixel_Sizes(face, 0, 48);

    FT_Load_Char(face, ch, FT_LOAD_RENDER);

    FT_Outline g = face->glyph->outline;

    FT_Vector* points = g.points;
    unsigned char* tags = g.tags;

    int currentPoint = 0;
    int pointsLeft = g.n_points;

    bool seenControlGroupStart = false;
    std::vector<simd_float2> renderedPoints {};
    std::vector<simd_float2> pointsBuffer {};

    while (pointsLeft > 0) {
        simd_float2 point = simd_float2{(float)points[currentPoint].x, (float)points[currentPoint].y};
        
        if (FT_CURVE_TAG(tags[currentPoint]) == FT_CURVE_TAG_ON) {
            if (seenControlGroupStart) {
                if (pointsBuffer.size() == 3) {
                    for (int t = 0; t <= resolution; ++t)
                        renderedPoints.push_back(quadraticInterpolation(pointsBuffer[0], pointsBuffer[1], pointsBuffer[2], t, resolution));
                    pointsBuffer.clear();
                    seenControlGroupStart = false;
                }else if (pointsBuffer.size() == 4) {
                    for (int t = 0; t <= resolution; ++t)
                        renderedPoints.push_back(cubicInterpolation(pointsBuffer[0], pointsBuffer[1], pointsBuffer[2], pointsBuffer[3], t, resolution));
                    pointsBuffer.clear();
                    seenControlGroupStart = false;
                }else {
                    renderedPoints.insert(renderedPoints.end(), pointsBuffer.begin(), pointsBuffer.end());
                    pointsBuffer.clear();
                }
                pointsBuffer.push_back(point);
            }else {
                seenControlGroupStart = true;
                pointsBuffer.push_back(point);
            }
        }else {
            pointsBuffer.push_back(point);
        }

        --pointsLeft;
        ++currentPoint;
    }

    FT_Done_Face(face);

    return renderedPoints;
}
