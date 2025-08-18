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
    textUniforms{.color=color, .numContours = 0},
    font{font},
    x{100.0},
    y{100.0},
    quadWidth{0.0},
    quadHeight{0.0}
{
    FT_New_Face(ft, font.c_str(), 0, &(this->face));
    FT_Set_Pixel_Sizes(face, 0, height);
    
    this->quadBuffer = this->device->newBuffer(sizeof(simd_float2)*4, MTL::StorageModeShared);
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
    std::vector<simd_float2> quadPoints {};
    std::vector<simd_float2> contourPoints {};
    std::vector<std::vector<simd_float2>> contours {};
    std::vector<std::pair<float,float>> offsets;
    std::vector<ContourMeta> contoursMeta {};

    float penX = FT_PIXEL_CF*x;
    float penY = FT_PIXEL_CF*(windowHeight-y);
    for (auto ch: this->text) {
        FT_Load_Char(face, ch, FT_LOAD_RENDER);
        FT_Outline* outlinePtr;
        
        if (outlineCache.find(ch) == outlineCache.end()) {
            FT_Outline* newOutline = new FT_Outline();
            FT_Load_Char(face, ch, FT_LOAD_RENDER);
            
            int err = FT_Outline_New(face->glyph->library, face->glyph->outline.n_points, face->glyph->outline.n_contours, newOutline);
            
            FT_Outline_Copy(&face->glyph->outline, newOutline);
            
            outlineCache[ch] = newOutline;
        }
        
        outlinePtr = outlineCache[ch];
        
        if (ch == '\r') {
            penY -= FT_PIXEL_CF*height;
            penX = FT_PIXEL_CF*x;
        }else {
            auto chContours = drawContours(outlinePtr, penX, penY);
            contours.insert(contours.end(), chContours.begin(), chContours.end());
            penX += this->face->glyph->advance.x;
        }
        
        offsets.push_back({penX, penY});
    }
    
    textUniforms.numContours = contours.size();

    float minX = std::numeric_limits<float>::infinity();
    float maxX = -std::numeric_limits<float>::infinity();
    float minY = std::numeric_limits<float>::infinity();
    float maxY = -std::numeric_limits<float>::infinity();

    unsigned long contourStart = 0;
    
    for (auto& contour: contours) {
        contourPoints.insert(contourPoints.end(), contour.begin(), contour.end());
        
        float minContourX = std::numeric_limits<float>::infinity();
        float maxContourX = -std::numeric_limits<float>::infinity();
        float minContourY = std::numeric_limits<float>::infinity();
        float maxContourY = -std::numeric_limits<float>::infinity();
        
        for (auto pt: contour) {
            if (pt.x < minX)
                minX = pt.x;
            if (pt.x < minContourX)
                minContourX = pt.x;

            if (pt.x > maxX)
                maxX = pt.x;
            if (pt.x > maxContourX)
                maxContourX = pt.x;

            if (pt.y < minY)
                minY = pt.y;
            if (pt.y < minContourY)
                minContourY = pt.y;

            if (pt.y > maxY)
                maxY = pt.y;
            if (pt.y > maxContourY)
                maxContourY = pt.y;
        }
 
        contoursMeta.push_back({.start = contourStart, .end = contourStart+contour.size(),
                                .minX = minContourX, .maxX=maxContourX,
                                .minY = minContourY, .maxY=maxContourY});
        
        contourStart += contour.size();
    }

    simd_float2 topLeft {minX, minY};
    simd_float2 topRight {maxX, minY};
    simd_float2 bottomLeft {minX, maxY};
    simd_float2 bottomRight {maxX, maxY};
    
    this->quadWidth = maxX-minX;
    this->quadHeight = maxY-minY;
    
    quadPoints.push_back(topLeft);
    quadPoints.push_back(topRight);
    quadPoints.push_back(bottomLeft);
    quadPoints.push_back(bottomRight);
    
    TextUniforms* uniformsBufferPtr = static_cast<TextUniforms*>(uniformsBuffer->contents());
    *uniformsBufferPtr = textUniforms;
    
    std::memcpy(this->quadBuffer->contents(), quadPoints.data(), 4*sizeof(simd_float2));

    resizeBuffer(&contoursBuffer, sizeof(simd_float2)*contourPoints.size());
    resizeBuffer(&contoursMetaBuffer, sizeof(ContourMeta)*contoursMeta.size());
    
    std::memcpy(this->contoursBuffer->contents(), contourPoints.data(), sizeof(simd_float2)*contourPoints.size());
    std::memcpy(this->contoursMetaBuffer->contents(), contoursMeta.data(), sizeof(ContourMeta)*contoursMeta.size());
}
