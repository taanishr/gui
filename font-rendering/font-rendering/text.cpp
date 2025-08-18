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

Text::Text(MTL::Device* device, FT_Library ft, float height, simd_float3 color, const std::string& font):
    device{device},
    height{height},
    textUniforms{.color=color},
    font{font},
    x{100.0},
    y{100.0},
    quadWidth{0.0},
    quadHeight{0.0},
    numQuads{0}
{
    FT_New_Face(ft, font.c_str(), 0, &(this->face));
    FT_Set_Pixel_Sizes(face, 0, height);
    
    this->quadBuffer = this->device->newBuffer(sizeof(Quad)*4, MTL::StorageModeShared);
    this->contoursBuffer = this->device->newBuffer(sizeof(simd_float2)*1024, MTL::StorageModeShared);
    this->uniformsBuffer = this->device->newBuffer(sizeof(TextUniforms), MTL::StorageModeShared);
    this->contoursMetaBuffer =  this->device->newBuffer(sizeof(ContourMeta)*4, MTL::StorageModeShared);
}

Text::~Text() {
    uniformsBuffer->release();
    quadBuffer->release();
    contoursBuffer->release();
    contoursMetaBuffer->release();
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
    
    // no need to copy contents
    MTL::Buffer* newBuffer = this->device->newBuffer(newLength, MTL::StorageModeShared);
    (*buffer)->release();
    (*buffer) = newBuffer;
}


void Text::update() {
    std::vector<Quad> quads {};
    std::vector<simd_float2> contourPoints {};
    std::vector<std::vector<simd_float2>> contours {};
    std::vector<std::pair<float,float>> offsets;
    std::vector<ContourMeta> contoursMeta {};

    float penX = FT_PIXEL_CF*x;
    float penY = FT_PIXEL_CF*(windowHeight-y);
    unsigned int firstContour = 0;
    unsigned int numQuadsInUpdate = 0;
    for (auto ch: this->text) {
        FT_Load_Char(face, ch, FT_LOAD_RENDER);
        
        FT_Outline* outlinePtr = &face->glyph->outline;
        
        if (ch == '\r') {
            penY -= FT_PIXEL_CF*height;
            penX = FT_PIXEL_CF*x;
        }else {
            auto chContoursAndQuad = drawContours(outlinePtr, penX, penY);
            // handle contours
            auto& chContours = std::get<0>(chContoursAndQuad);
            contours.insert(contours.end(), chContours.begin(), chContours.end());
            unsigned int lastContour = firstContour + static_cast<unsigned int>(chContours.size());
            
            // handle quad
            auto topLeft = std::get<1>(chContoursAndQuad);
            auto bottomRight = std::get<2>(chContoursAndQuad);
            quads.push_back({.position=topLeft, .offset={penX,penY},.firstContour = firstContour, .lastContour = lastContour});
            quads.push_back({.position={bottomRight.x, topLeft.y}, .offset={penX,penY}, .firstContour = firstContour, .lastContour = lastContour});
            quads.push_back({.position={topLeft.x, bottomRight.y}, .offset={penX,penY}, .firstContour = firstContour, .lastContour = lastContour});
            quads.push_back({.position={topLeft.x, bottomRight.y}, .offset={penX,penY}, .firstContour = firstContour, .lastContour = lastContour});
            quads.push_back({.position={bottomRight.x, topLeft.y}, .offset={penX,penY}, .firstContour = firstContour, .lastContour = lastContour});
            quads.push_back({.position=bottomRight, .offset={penX,penY}, .firstContour = firstContour, .lastContour = lastContour});
            ++numQuadsInUpdate;
            
            firstContour = lastContour;
            
            penX += this->face->glyph->advance.x;
        }
        
        offsets.push_back({penX, penY});
    }
    
    this->numQuads = numQuadsInUpdate;
    
    std::println("num quads: {}", numQuads);
    
    unsigned long contourStart = 0;
    
    for (auto& contour: contours) {
        contourPoints.insert(contourPoints.end(), contour.begin(), contour.end());
        
        std::println("num points per contour: {}", contour.size());
    
        contoursMeta.push_back({.start = contourStart, .end = contourStart+contour.size()});
        
        contourStart += contour.size();
    }
    
    TextUniforms* uniformsBufferPtr = static_cast<TextUniforms*>(uniformsBuffer->contents());
    *uniformsBufferPtr = textUniforms;
    
    std::println("num points: {}", contourPoints.size());

    resizeBuffer(&quadBuffer, sizeof(Quad)*quads.size());
    resizeBuffer(&contoursBuffer, sizeof(simd_float2)*contourPoints.size());
    resizeBuffer(&contoursMetaBuffer, sizeof(ContourMeta)*contoursMeta.size());
    
    std::memcpy(this->quadBuffer->contents(), quads.data(), quads.size()*sizeof(Quad));
    std::memcpy(this->contoursBuffer->contents(), contourPoints.data(), sizeof(simd_float2)*contourPoints.size());
    std::memcpy(this->contoursMetaBuffer->contents(), contoursMeta.data(), sizeof(ContourMeta)*contoursMeta.size());
}
