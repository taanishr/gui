//
//  text.cpp
//  font-rendering
//
//  Created by Taanish Reja on 8/15/25.
//

// optimization tricks to try:
// - quad per glpyh?

#include "text.hpp"

void printPoint(const simd_float2& pt, bool newLine) {
    std::string end = newLine ? "\n" : "";
    std::print("({},{}){}", pt.x, pt.y, end);
}

Text::Text(MTL::Device* device, FT_Library ft, float fontSize, simd_float3 color, const std::string& font):
    device{device},
    fontSize{fontSize},
    color{color},
    font{font},
    x{100.0},
    y{100.0},
    lastBezierPoint{0},
    numQuadPoints{0}
{
    FT_New_Face(ft, font.c_str(), 0, &(this->face));
    FT_Set_Pixel_Sizes(face, 0, fontSize);
    
    this->quadBuffer = device->newBuffer(sizeof(QuadPoint)*256, MTL::ResourceStorageModeShared);
    this->bezierPointsBuffer = device->newBuffer(sizeof(simd_float2)*112, MTL::ResourceStorageModeShared);
    this->uniformsBuffer = device->newBuffer(sizeof(Uniforms)*1, MTL::ResourceStorageModeShared);
    this->glyphMetaBuffer = device->newBuffer(sizeof(int)*256, MTL::ResourceStorageModeShared);
}

Text::~Text() {
    this->glyphMetaBuffer->release();
    this->uniformsBuffer->release();
    this->bezierPointsBuffer->release();
    this->quadBuffer->release();
    FT_Done_Face(face);
}

void Text::setText(const std::string& text) {
    this->text = text;
}

void Text::resizeBuffer(MTL::Buffer** buffer, unsigned long numBytes) {
    unsigned long oldLength = (*buffer)->length();

    if (numBytes < oldLength)
        return;

    unsigned long newLength = std::max(oldLength*2, numBytes);

    MTL::Buffer* newBuffer = this->device->newBuffer(newLength, MTL::StorageModeShared);
    (*buffer)->release();
    (*buffer) = newBuffer;
}

void Text::update() {
    std::vector<QuadPoint> quadPoints;
    std::vector<int> glyphMeta;
    
    int metadataIndex = 0;
    int bezierIndex = lastBezierPoint;
    
    simd_float2 drawOffset {this->x*64.0f, (windowHeight-this->y)*64.0f};
    for (auto ch : text) {
        if (ch == '\r') {
            drawOffset.y -= this->fontSize*64.0f;
            drawOffset.x = this->x*64.0f;
            continue;
        }
            
        FT_Load_Char(this->face, ch, FT_LOAD_NO_BITMAP | FT_LOAD_NO_HINTING | FT_LOAD_NO_AUTOHINT);
        
        auto glyph = this->face->glyph;
        auto outline = &this->face->glyph->outline;
        
        bool contoursCached = glyphBezierMap.find(ch) != glyphBezierMap.end();
        
        if (!contoursCached) {
            auto processedGlyph = processContours(outline, bezierIndex);
            
            // handle quad
            auto quad = processedGlyph.quad;
            
            // cache points metadata and glyph
            glyphBezierMap[ch] = bezierIndex;
            glyphMap[ch] = processedGlyph;
            
            quadPoints.push_back({
                .position = quad.topLeft,
                .offset = drawOffset,
                .metadataIndex = metadataIndex
            });
            
            quadPoints.push_back({
                .position = {quad.bottomRight.x, quad.topLeft.y},
                .offset = drawOffset,
                .metadataIndex = metadataIndex
            });
            
            quadPoints.push_back({
                .position = {quad.topLeft.x, quad.bottomRight.y},
                .offset = drawOffset,
                .metadataIndex = metadataIndex
            });
            
            quadPoints.push_back({
                .position = {quad.topLeft.x, quad.bottomRight.y},
                .offset = drawOffset,
                .metadataIndex = metadataIndex
            });
            
            
            quadPoints.push_back({
                .position = {quad.bottomRight.x, quad.topLeft.y},
                .offset = drawOffset,
                .metadataIndex = metadataIndex
            });
            
            quadPoints.push_back({
                .position = quad.bottomRight,
                .offset = drawOffset,
                .metadataIndex = metadataIndex
            });
            
        
            // handle metadata
            glyphMeta.push_back(bezierIndex); // bezier index; pull from map
            glyphMeta.push_back(processedGlyph.numContours); // numContours
            for (int contourSize : processedGlyph.contourSizes) // num points per contour
                glyphMeta.push_back(contourSize);
            
            metadataIndex += 2 + processedGlyph.contourSizes.size();
            
            // handle points
            bezierPoints.insert(bezierPoints.end(), processedGlyph.points.begin(), processedGlyph.points.end());
            
            // increment counters
            lastBezierPoint = bezierIndex + processedGlyph.points.size();
        }else {
            bezierIndex = glyphBezierMap[ch];
            
            auto& processedGlyph = glyphMap[ch];
            
            // handle quad
            auto quad = processedGlyph.quad;
            
            quadPoints.push_back({
                .position = quad.topLeft,
                .offset = drawOffset,
                .metadataIndex = metadataIndex
            });
            
            quadPoints.push_back({
                .position = {quad.bottomRight.x, quad.topLeft.y},
                .offset = drawOffset,
                .metadataIndex = metadataIndex
            });
            
            quadPoints.push_back({
                .position = {quad.topLeft.x, quad.bottomRight.y},
                .offset = drawOffset,
                .metadataIndex = metadataIndex
            });
            
            quadPoints.push_back({
                .position = {quad.topLeft.x, quad.bottomRight.y},
                .offset = drawOffset,
                .metadataIndex = metadataIndex
            });
            
            
            quadPoints.push_back({
                .position = {quad.bottomRight.x, quad.topLeft.y},
                .offset = drawOffset,
                .metadataIndex = metadataIndex
            });
            
            quadPoints.push_back({
                .position = quad.bottomRight,
                .offset = drawOffset,
                .metadataIndex = metadataIndex
            });
            
            // handle metadata
            glyphMeta.push_back(bezierIndex); // bezier index; pull from map
            glyphMeta.push_back(processedGlyph.numContours); // numContours
            for (int contourSize : processedGlyph.contourSizes) // num points per contour
                glyphMeta.push_back(contourSize);
            
            metadataIndex += 2 + processedGlyph.contourSizes.size();
        }
        
        drawOffset.x += glyph->metrics.horiAdvance;
        bezierIndex = lastBezierPoint;
    }
    
    this->numQuadPoints = quadPoints.size();
    
    // copy uniforms buffer
    Uniforms uniforms {.color=this->color};
    std::memcpy(this->uniformsBuffer->contents(), &uniforms, sizeof(Uniforms));
    
    // copy resizable buffers
    resizeBuffer(&quadBuffer, quadPoints.size()*sizeof(QuadPoint));
    resizeBuffer(&bezierPointsBuffer, bezierPoints.size()*sizeof(simd_float2));
    resizeBuffer(&glyphMetaBuffer, glyphMeta.size()*sizeof(int));

    std::memcpy(this->quadBuffer->contents(), quadPoints.data(), quadPoints.size()*sizeof(QuadPoint));
    std::memcpy(this->bezierPointsBuffer->contents(), bezierPoints.data(), bezierPoints.size()*sizeof(simd_float2));
    std::memcpy(this->glyphMetaBuffer->contents(), glyphMeta.data(), glyphMeta.size()*sizeof(int));
}
