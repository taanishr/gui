//
//  text.cpp
//  font-rendering
//
//  Created by Taanish Reja on 8/15/25.
//

#include "text.hpp"
#include "Renderer.hpp"

using namespace TextRender;

Text::Text(Renderer& renderer, float x, float y, float fontSize, simd_float4 color, const std::string& font):
    renderer{renderer},
    glyphCache{renderer.glyphCache()},
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
    this->quadOffsetBuffer = renderer.device->newBuffer(sizeof(simd_float2)*256, MTL::ResourceStorageModeShared);
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
    this->quadOffsetBuffer->release();
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
    
    vertexDescriptor->attributes()->object(1)->setFormat(MTL::VertexFormat::VertexFormatInt);
    vertexDescriptor->attributes()->object(1)->setOffset(sizeof(simd_float2));
    vertexDescriptor->attributes()->object(1)->setBufferIndex(0);
    
    vertexDescriptor->attributes()->object(2)->setFormat(MTL::VertexFormat::VertexFormatInt);
    vertexDescriptor->attributes()->object(2)->setOffset(sizeof(simd_float2) + sizeof(int));
    vertexDescriptor->attributes()->object(2)->setBufferIndex(0);
    
//
//    vertexDescriptor->attributes()->object(1)->setFormat(MTL::VertexFormat::VertexFormatFloat2);
//    vertexDescriptor->attributes()->object(1)->setOffset(sizeof(simd_float2));
//    vertexDescriptor->attributes()->object(1)->setBufferIndex(0);
//    
//    vertexDescriptor->attributes()->object(2)->setFormat(MTL::VertexFormat::VertexFormatInt);
//    vertexDescriptor->attributes()->object(2)->setOffset(sizeof(simd_float2)*2);
//    vertexDescriptor->attributes()->object(2)->setBufferIndex(0);

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

void Text::update(const TextLayout& layoutBox) {
    this->textLayout = &layoutBox;
    
    std::vector<QuadPoint> quadPoints;
    std::vector<simd_float2> quadOffsets;
    std::vector<int> glyphMeta;
    
    int metadataIndex = 0;
    int bezierIndex = lastBezierPoint;
    
    Uniforms uniforms {.color=this->color};
    auto frameInfo = renderer.getFrameInfo();
    simd_float2 drawOffset {layoutBox.computedX*FT_PIXEL_CF, (layoutBox.computedY+fontSize)*FT_PIXEL_CF};
    
    int ani = 0;
    
    for (auto ch : text) {
        if (ch == '\r') {
            drawOffset.y += this->fontSize*FT_PIXEL_CF;
            drawOffset.x = layoutBox.computedX*FT_PIXEL_CF;
            continue;
        }
        
        bool contoursCached = glyphBezierMap.find(ch) != glyphBezierMap.end();
        
        if (!contoursCached) {
            auto glyph = glyphCache.retrieve(font, fontSize, ch);
            
            // cache points metadata and glyph
            glyphBezierMap[ch] = bezierIndex;
            glyphMap[ch] = glyph;
    
            // inset points
            bezierPoints.insert(bezierPoints.end(), glyph.points.begin(), glyph.points.end());
            
            // increment counter (change this to be unsigned long/size type so it doesn't run into weird bugs in the future)
            lastBezierPoint = bezierIndex + glyph.points.size();
        }
        
        bezierIndex = glyphBezierMap[ch];
        auto& glyph = glyphMap[ch];
        
        // handle quad
        auto quad = glyph.quad;

        quadPoints.push_back({
            .position = quad.topLeft,
            .glyphIndex = ani,
            .metadataIndex = metadataIndex
        });
        
        quadPoints.push_back({
            .position = {quad.bottomRight.x, quad.topLeft.y},
            .glyphIndex = ani,
            .metadataIndex = metadataIndex
        });
        
        quadPoints.push_back({
            .position = {quad.topLeft.x, quad.bottomRight.y},
            .glyphIndex = ani,
            .metadataIndex = metadataIndex
        });
        
        quadPoints.push_back({
            .position = {quad.topLeft.x, quad.bottomRight.y},
            .glyphIndex = ani,
            .metadataIndex = metadataIndex
        });
        
        
        quadPoints.push_back({
            .position = {quad.bottomRight.x, quad.topLeft.y},
            .glyphIndex = ani,
            .metadataIndex = metadataIndex
        });
        
        quadPoints.push_back({
            .position = quad.bottomRight,
            .glyphIndex = ani,
            .metadataIndex = metadataIndex
        });
        
        
        if (not fragmented)
            quadOffsets.push_back(drawOffset);
        
        ani += 1;
        
        // handle metadata
        glyphMeta.push_back(bezierIndex); // bezier index; pull from map
        glyphMeta.push_back(glyph.numContours); // numContours
        for (int contourSize : glyph.contourSizes) // num points per contour
            glyphMeta.push_back(contourSize);
        
        metadataIndex += 2 + glyph.contourSizes.size();
    
        
        drawOffset.x += glyph.metrics.horiAdvance;
        bezierIndex = lastBezierPoint;
    }
    
    this->numQuadPoints = quadPoints.size();
    
    // copy uniforms buffer
    std::memcpy(this->uniformsBuffer->contents(), &uniforms, sizeof(Uniforms));
    
    // copy frame info buffer
    std::memcpy(this->frameInfoBuffer->contents(), &frameInfo, sizeof(FrameInfo));
    
    
    // copy resizable buffers
    resizeBuffer(renderer.device, quadBuffer, quadPoints.size()*sizeof(QuadPoint));
    
    if (not fragmented)
        resizeBuffer(renderer.device, quadOffsetBuffer, quadOffsets.size()*sizeof(simd_float2));
    
    resizeBuffer(renderer.device, bezierPointsBuffer, bezierPoints.size()*sizeof(simd_float2));
    resizeBuffer(renderer.device, glyphMetaBuffer, glyphMeta.size()*sizeof(int));

    std::memcpy(this->quadBuffer->contents(), quadPoints.data(), quadPoints.size()*sizeof(QuadPoint));
    
    if (not fragmented) {
        std::memcpy(this->quadOffsetBuffer->contents(), quadOffsets.data(), quadOffsets.size()*sizeof(simd_float2));
    }

    std::memcpy(this->bezierPointsBuffer->contents(), bezierPoints.data(), bezierPoints.size()*sizeof(simd_float2));
    std::memcpy(this->glyphMetaBuffer->contents(), glyphMeta.data(), glyphMeta.size()*sizeof(int));
}

const TextLayout& Text::layout() const
{
    return *(this->textLayout);
}

FragmentTemplate& _imeasure(Constraints& constraints)
{
    // vector<Atom> atoms;
    // float width;
    // float height;
    // loop over characters; for now, we're not handling ligatures. treat every char as an "atom'
        // if new line
            // height += line height
            // continue
    
        // retrieve glyph
        // create atom (h/w of glyph)
        // width += atom width
    
    // instantiate fragment template (atoms, width, height)
    // return FragmenTemplate
}

const DrawableSize& Text::measure()
{
    float currW = 0, maxW = 0, maxH = this->fontSize;
    
    for (auto ch : text) {
        if (ch == '\r') {
            maxH += this->fontSize;
            currW = 0;
            continue;
        }
        
        auto glyph = glyphCache.retrieve(font, fontSize, ch);
        
        currW += glyph.metrics.horiAdvance / FT_PIXEL_CF;
        maxW = std::max(maxW, currW);
    }
    
    intrinsicSize.width = maxW;
    intrinsicSize.height = maxH;
    
    return intrinsicSize;
}

bool Text::contains(simd_float2 point) const
{
    simd_float2 topLeft {this->textLayout->x, this->textLayout->y};
    simd_float2 bottomRight {this->textLayout->x + this->textLayout->computedWidth, this->textLayout->y + this->textLayout->computedHeight};
    
    return !(point.x < topLeft.x || point.x > bottomRight.x ||
             point.y < topLeft.y || point.y > bottomRight.y);
}

void Text::encode(MTL::RenderCommandEncoder* encoder) {
//    auto framePixelSize = renderer.getFramePixelSize();
//    encoder->setScissorRect({0,0, static_cast<unsigned long>(framePixelSize.width), static_cast<unsigned long>(framePixelSize.height)});
    
    auto pipeline = getPipeline();
    encoder->setRenderPipelineState(pipeline);
    encoder->setVertexBuffer(this->quadBuffer, 0, 0);
    encoder->setVertexBuffer(this->quadOffsetBuffer, 0, 1);
    encoder->setVertexBuffer(this->frameInfoBuffer, 0, 2);

    encoder->setFragmentBuffer(this->bezierPointsBuffer, 0, 0);
    encoder->setFragmentBuffer(this->glyphMetaBuffer, 0, 1);
    encoder->setFragmentBuffer(this->uniformsBuffer, 0, 2);
    
    encoder->drawPrimitives(MTL::PrimitiveType::PrimitiveTypeTriangle, NS::UInteger(0), NS::UInteger(this->numQuadPoints));
}

// encodeFragment -> encodes a fragment (start and end offset)
// what do I want to do?
// - opt 1: make a fragment data structure
// - contains vector of pairs (offsets for each fragment)
// - what does contains do? calls measure
// - encode... maybe I can overload it to encode fragments?
// - and maybe I can overload contains to encode fragments?
// maybe each node can store whether it is fragmented?

// or I can make it so fragments are basically invisible to the render tree?
// that makes it very easy to transistion (e.g. for dispatch, contains knows what to do)
// but how does the layout fragment the node?

std::optional<simd_float2> Text::measureFragment(int startingIndex, int endingIndex)
{
    if (startingIndex < 0 || endingIndex >= text.size())
        return std::nullopt;
    
    float currW = 0, maxW = 0, maxH = this->fontSize;
    
    std::string_view tv {text};
    
    auto stv = tv.substr(startingIndex, endingIndex-startingIndex);
    
    std::println("{}", stv);
    
    for (auto ch : stv) {
        if (ch == '\r') {
            maxH += this->fontSize;
            currW = 0;
            continue;
        }
        
        auto glyph = glyphCache.retrieve(font, fontSize, ch);
        
        currW += glyph.metrics.horiAdvance / FT_PIXEL_CF;
        maxW = std::max(maxW, currW);
    }
    
    return simd_float2{maxW, maxH};
};


// how will this be used? It needs to be returned a max width and height, a starting flow x and y, then it needs to compute the constraints
void Text::fragment(float flowX, float flowY, float maxWidth, float maxHeight) {
    std::vector<simd_float2> quadOffsets;
    
    int sInd = 0;
    int eInd = 1;
    
    float currFragmentX = flowX, currFragmentY = flowY;
    

    while (eInd < text.size()) {
        auto fs = measureFragment(sInd, eInd);

        if (not fs.has_value())
            break;
        
        
        std::println("eind: {}", eInd);
        

//        std::println("currFragmentX: {} fs->x: {}, currFragmentX + fs->x: {} mw: {}", currFragmentX, static_cast<float>(fs->x), currFragmentX + fs->x, maxWidth);
        
        if (currFragmentX + fs->x >= maxWidth) {
            

            
            for (auto ch : std::string_view(this->text).substr(sInd, eInd-1)) {
                if (ch == '\r') {
                    continue;
                }
                
                quadOffsets.push_back(simd_float2{currFragmentX, currFragmentY});
                
                auto glyph = glyphCache.retrieve(font, fontSize, ch);
                
                currFragmentX += glyph.metrics.horiAdvance / FT_PIXEL_CF;;
            }
        
            currFragmentX = 0;
            currFragmentY += fontSize;
            sInd = eInd;
            
        }
        
//        if (currFragmentY >= maxHeight) {
//            break;
//        }
        
        ++eInd;
    }
    
    
    resizeBuffer(this->renderer.device, this->quadOffsetBuffer, quadOffsets.size()*sizeof(simd_float2));
    std::memcpy(this->quadOffsetBuffer->contents(), quadOffsets.data(), quadOffsets.size()*sizeof(simd_float2));
    
    this->fragmented = true;
}

void Text::defragment() {
    this->fragmented = false;
}

