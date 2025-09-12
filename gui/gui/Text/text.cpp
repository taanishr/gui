//
//  text.cpp
//  font-rendering
//
//  Created by Taanish Reja on 8/15/25.
//

#include "text.hpp"
#include "Renderer.hpp"

using namespace TextRender;

std::size_t GlyphFaceHash::operator()(const std::pair<FontName, FontSize>& cacheKey) const
{
    std::size_t hv = 0;
    
    hash_combine(hv, cacheKey.first);
    hash_combine(hv, cacheKey.second);
    
    return hv;
}

std::size_t GlyphCacheHash::operator()(const std::tuple<FontName, FontSize, char>& fontKey) const
{
    std::size_t hv = 0;
    
    hash_combine(hv, std::get<0>(fontKey));
    hash_combine(hv, std::get<1>(fontKey));
    hash_combine(hv, std::get<2>(fontKey));
    
    return hv;
}

GlyphCache::GlyphCache(FT_Library ft):
    ft{ft}
{}


GlyphCache::~GlyphCache() {
    for (auto& pair : fontFaces)
        FT_Done_Face(pair.second);
}

const Glyph& GlyphCache::retrieve(const FontName& font, FontSize fontSize, char ch)
{
    bool glyphCached = cache.find({font,fontSize,ch}) == cache.end();

    if (!glyphCached) {
        bool faceCached = fontFaces.find({font, fontSize}) == fontFaces.end();
        
        if (!faceCached) {
            FT_Face newFace = nullptr;
            FT_New_Face(this->ft, font.c_str(), 0, &newFace);
            FT_Set_Pixel_Sizes(newFace, 0, fontSize);
            
            fontFaces[{font, fontSize}] = newFace;
        }
        
        auto fontFace = fontFaces[{font, fontSize}];

        FT_Load_Char(fontFace, ch, FT_LOAD_NO_BITMAP | FT_LOAD_NO_HINTING | FT_LOAD_NO_AUTOHINT);

        auto outline = &fontFace->glyph->outline;

        auto processedGlyph = processContours(outline, 0);
    
        cache[{font, fontSize, ch}] = processedGlyph;
    }

    auto glyphIt = cache.find({font,fontSize,ch});

    return glyphIt->second;
}

Text::Text(Renderer& renderer, float x, float y, float fontSize, simd_float4 color, const std::string& font):
    renderer{renderer},
    x{x},
    y{y},
    fontSize{fontSize},
    color{color},
    font{font},
    lastBezierPoint{0},
    numQuadPoints{0}
{
    FT_New_Face(renderer.ft, font.c_str(), 0, &(this->face));
    FT_Set_Pixel_Sizes(face, 0, fontSize);
    
    this->quadBuffer = renderer.device->newBuffer(sizeof(QuadPoint)*256, MTL::ResourceStorageModeShared);
    this->bezierPointsBuffer = renderer.device->newBuffer(sizeof(simd_float2)*112, MTL::ResourceStorageModeShared);
    this->uniformsBuffer = renderer.device->newBuffer(sizeof(Uniforms), MTL::ResourceStorageModeShared);
    this->glyphMetaBuffer = renderer.device->newBuffer(sizeof(int)*256, MTL::ResourceStorageModeShared);
    this->frameInfoBuffer = renderer.device->newBuffer(sizeof(FrameInfo), MTL::ResourceStorageModeShared);
}

Text::~Text() {
    this->frameInfoBuffer->release();
    this->glyphMetaBuffer->release();
    this->uniformsBuffer->release();
    this->bezierPointsBuffer->release();
    this->quadBuffer->release();
    FT_Done_Face(face);
}

void Text::buildPipeline(MTL::RenderPipelineState*& pipeline) {
    MTL::Library* defaultLibrary = renderer.device->newDefaultLibrary();
    MTL::RenderPipelineDescriptor* renderPipelineDescriptor = MTL::RenderPipelineDescriptor::alloc()->init();
    
    // set up vertex descriptor
    MTL::VertexDescriptor* vertexDescriptor = MTL::VertexDescriptor::alloc()->init();
    vertexDescriptor->attributes()->object(0)->setFormat(MTL::VertexFormat::VertexFormatFloat2);
    vertexDescriptor->attributes()->object(0)->setOffset(0);
    vertexDescriptor->attributes()->object(0)->setBufferIndex(0);
    
    vertexDescriptor->attributes()->object(1)->setFormat(MTL::VertexFormat::VertexFormatFloat2);
    vertexDescriptor->attributes()->object(1)->setOffset(sizeof(simd_float2));
    vertexDescriptor->attributes()->object(1)->setBufferIndex(0);
    
    vertexDescriptor->attributes()->object(2)->setFormat(MTL::VertexFormat::VertexFormatInt);
    vertexDescriptor->attributes()->object(2)->setOffset(sizeof(simd_float2)*2);
    vertexDescriptor->attributes()->object(2)->setBufferIndex(0);

    vertexDescriptor->layouts()->object(0)->setStride(sizeof(QuadPoint));
    
    renderPipelineDescriptor->setVertexDescriptor(vertexDescriptor);

    
    // set up vertex function
    MTL::Function* vertexFunction = defaultLibrary->newFunction(NS::String::string("vertex_text", NS::UTF8StringEncoding));
    renderPipelineDescriptor->setVertexFunction(vertexFunction);
    
    renderPipelineDescriptor->colorAttachments()->object(0)->setPixelFormat(renderer.view->colorPixelFormat());
    renderPipelineDescriptor->colorAttachments()->object(0)->setBlendingEnabled(true);
    renderPipelineDescriptor->colorAttachments()->object(0)->setAlphaBlendOperation(MTL::BlendOperationAdd);
    renderPipelineDescriptor->colorAttachments()->object(0)->setSourceRGBBlendFactor(MTL::BlendFactorSourceAlpha);
    renderPipelineDescriptor->colorAttachments()->object(0)->setDestinationRGBBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
    renderPipelineDescriptor->colorAttachments()->object(0)->setSourceAlphaBlendFactor(MTL::BlendFactorSourceAlpha);
    renderPipelineDescriptor->colorAttachments()->object(0)->setDestinationAlphaBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
    
    
    renderPipelineDescriptor->setDepthAttachmentPixelFormat(renderer.view->depthStencilPixelFormat());
    

    // set up fragment function
    MTL::Function* fragmentFunction = defaultLibrary->newFunction(NS::String::string("fragment_text", NS::UTF8StringEncoding));
    renderPipelineDescriptor->setFragmentFunction(fragmentFunction);
    
    
    NS::Error* error = nullptr;
    pipeline = renderer.device->newRenderPipelineState(renderPipelineDescriptor, &error);
    
    if (error != nullptr)
        std::println("error in pipeline creation: {}", error->localizedDescription()->utf8String());
    
    
    defaultLibrary->release();
    renderPipelineDescriptor->release();
    vertexDescriptor->release();
    vertexFunction->release();
}

MTL::RenderPipelineState* Text::getPipeline() {
    static MTL::RenderPipelineState* pipeline = nullptr;
    
    if (!pipeline)
        buildPipeline(pipeline);
    
    return pipeline;
}

void Text::setText(const std::string& text) {
    this->text = text;
}

void Text::addChar(char ch) {
    this->text += ch;
}

void Text::removeChar() {
    if (this->text.size() > 0)
        this->text.pop_back();
}

void Text::update(const LayoutBox& layoutBox) {
    this->textLayout = &layoutBox;
    
    std::vector<QuadPoint> quadPoints;
    std::vector<int> glyphMeta;
    
    int metadataIndex = 0;
    int bezierIndex = lastBezierPoint;
    
    Uniforms uniforms {.color=this->color};
    auto frameInfo = renderer.getFrameInfo();
    simd_float2 drawOffset {layoutBox.x*64.0f, (frameInfo.height-fontSize-layoutBox.y)*64.0f};
    
    for (auto ch : text) {
        if (ch == '\r') {
            drawOffset.y -= this->fontSize*64.0f;
            drawOffset.x = layoutBox.x*64.0f;
            continue;
        }
            
        FT_Load_Char(this->face, ch, FT_LOAD_NO_BITMAP | FT_LOAD_NO_HINTING | FT_LOAD_NO_AUTOHINT);
        
        auto glyph = this->face->glyph;
        auto outline = &this->face->glyph->outline;
        
        bool contoursCached = glyphBezierMap.find(ch) != glyphBezierMap.end();
        
        if (!contoursCached) {
            auto processedGlyph = processContours(outline, bezierIndex);
            
            // cache points metadata and glyph
            glyphBezierMap[ch] = bezierIndex;
            glyphMap[ch] = processedGlyph;
    
            // inset points
            bezierPoints.insert(bezierPoints.end(), processedGlyph.points.begin(), processedGlyph.points.end());
            
            // increment counter
            lastBezierPoint = bezierIndex + processedGlyph.points.size();
        }
        
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
    
        
        drawOffset.x += glyph->metrics.horiAdvance;
        bezierIndex = lastBezierPoint;
    }
    
    this->numQuadPoints = quadPoints.size();
    
    if (this->numQuadPoints > 0)
        this->elementBounds = {.topLeft = quadPoints.front().position, .bottomRight = quadPoints.back().position};
    
    // copy uniforms buffer
    std::memcpy(this->uniformsBuffer->contents(), &uniforms, sizeof(Uniforms));
    
    // copy frame info buffer
    std::memcpy(this->frameInfoBuffer->contents(), &frameInfo, sizeof(FrameInfo));
    
    
    // copy resizable buffers
    resizeBuffer(renderer.device, quadBuffer, quadPoints.size()*sizeof(QuadPoint));
    resizeBuffer(renderer.device, bezierPointsBuffer, bezierPoints.size()*sizeof(simd_float2));
    resizeBuffer(renderer.device, glyphMetaBuffer, glyphMeta.size()*sizeof(int));

    std::memcpy(this->quadBuffer->contents(), quadPoints.data(), quadPoints.size()*sizeof(QuadPoint));
    std::memcpy(this->bezierPointsBuffer->contents(), bezierPoints.data(), bezierPoints.size()*sizeof(simd_float2));
    std::memcpy(this->glyphMetaBuffer->contents(), glyphMeta.data(), glyphMeta.size()*sizeof(int));
}

//
//const Bounds& Text::bounds() const
//{
//    return elementBounds;
//}

const LayoutBox& Text::layout() const {
    return *(this->textLayout);
}

const DrawableSize& Text::measure() const {
    return intrinsicSize;
}



bool Text::contains(simd_float2 point) const
{
    return point.x < elementBounds.topLeft.x || point.x > elementBounds.bottomRight.x ||
    point.y < elementBounds.topLeft.y || point.y > elementBounds.bottomRight.y;
}

void Text::encode(MTL::RenderCommandEncoder* encoder) {
//    auto framePixelSize = renderer.getFramePixelSize();
//    encoder->setScissorRect({0,0, static_cast<unsigned long>(framePixelSize.width), static_cast<unsigned long>(framePixelSize.height)});
    
    auto pipeline = getPipeline();
    encoder->setRenderPipelineState(pipeline);
    encoder->setVertexBuffer(this->quadBuffer, 0, 0);
    encoder->setVertexBuffer(this->frameInfoBuffer, 0, 1);

    encoder->setFragmentBuffer(this->bezierPointsBuffer, 0, 0);
    encoder->setFragmentBuffer(this->glyphMetaBuffer, 0, 1);
    encoder->setFragmentBuffer(this->uniformsBuffer, 0, 2);
    
    encoder->drawPrimitives(MTL::PrimitiveType::PrimitiveTypeTriangle, NS::UInteger(0), NS::UInteger(this->numQuadPoints));
}
