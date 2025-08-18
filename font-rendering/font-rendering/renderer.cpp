//
//  renderer.cpp
//  font-rendering
//
//  Created by Taanish Reja on 7/21/25.
//

#include "renderer.hpp"
#include <iostream>
#include <ranges>

float linearInterpolation(float x1, float y1, float x2, float y2) {
    return (y2-y1)/(x2-x1);
}

Renderer::Renderer(MTL::Device* device, MTK::View* view):
    device{device},
    view{view},
    commandQueue(device->newCommandQueue()),
    frameSemaphore{1}
{
    makePipeline();
    makeResources();
}

void Renderer::makePipeline() {
    MTL::Library* defaultLibrary = device->newDefaultLibrary();
    MTL::RenderPipelineDescriptor* renderPipelineDescriptor = MTL::RenderPipelineDescriptor::alloc()->init();
    
    // set up vertex descriptor
    MTL::VertexDescriptor* vertexDescriptor = MTL::VertexDescriptor::alloc()->init();
    vertexDescriptor->attributes()->object(0)->setFormat(MTL::VertexFormat::VertexFormatFloat2);
    vertexDescriptor->attributes()->object(0)->setOffset(0);
    vertexDescriptor->attributes()->object(0)->setBufferIndex(0);
    
    vertexDescriptor->attributes()->object(1)->setFormat(MTL::VertexFormat::VertexFormatFloat2);
    vertexDescriptor->attributes()->object(1)->setOffset(sizeof(simd_float2));
    vertexDescriptor->attributes()->object(1)->setBufferIndex(0);
    
    vertexDescriptor->attributes()->object(2)->setFormat(MTL::VertexFormat::VertexFormatUInt);
    vertexDescriptor->attributes()->object(2)->setOffset(sizeof(simd_float2)*2);
    vertexDescriptor->attributes()->object(2)->setBufferIndex(0);
    
    vertexDescriptor->attributes()->object(3)->setFormat(MTL::VertexFormat::VertexFormatUInt);
    vertexDescriptor->attributes()->object(3)->setOffset(sizeof(simd_float2)*2+sizeof(unsigned int));
    vertexDescriptor->attributes()->object(3)->setBufferIndex(0);

    vertexDescriptor->layouts()->object(0)->setStride(sizeof(Quad));
    
    renderPipelineDescriptor->setVertexDescriptor(vertexDescriptor);

    
    // set up vertex function
    MTL::Function* vertexFunction = defaultLibrary->newFunction(NS::String::string("vertex_main", NS::UTF8StringEncoding));
    renderPipelineDescriptor->setVertexFunction(vertexFunction);
    
    renderPipelineDescriptor->colorAttachments()->object(0)->setPixelFormat(this->view->colorPixelFormat());
    renderPipelineDescriptor->colorAttachments()->object(0)->setBlendingEnabled(true);
    renderPipelineDescriptor->colorAttachments()->object(0)->setAlphaBlendOperation(MTL::BlendOperationAdd);
    renderPipelineDescriptor->colorAttachments()->object(0)->setSourceRGBBlendFactor(MTL::BlendFactorSourceAlpha);
    renderPipelineDescriptor->colorAttachments()->object(0)->setDestinationRGBBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
    renderPipelineDescriptor->colorAttachments()->object(0)->setSourceAlphaBlendFactor(MTL::BlendFactorSourceAlpha);
    renderPipelineDescriptor->colorAttachments()->object(0)->setDestinationAlphaBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
    
    
    renderPipelineDescriptor->setDepthAttachmentPixelFormat(this->view->depthStencilPixelFormat());
    

    // set up fragment function
    MTL::Function* fragmentFunction = defaultLibrary->newFunction(NS::String::string("fragment_main", NS::UTF8StringEncoding));
    renderPipelineDescriptor->setFragmentFunction(fragmentFunction);
    
    
    NS::Error* error = nullptr;
    this->renderPipelineState = this->device->newRenderPipelineState(renderPipelineDescriptor, &error);
    
    if (error != nullptr)
        std::println("error in pipeline creation: {}", error->localizedDescription()->utf8String());
    
    
    defaultLibrary->release();
    renderPipelineDescriptor->release();
    vertexDescriptor->release();
    vertexFunction->release();    
}

void Renderer::makeResources() {
    FT_Init_FreeType(&(this->ft));
    textBlocks.push_back(std::make_unique<Text>(device, ft, 16.0));
    textBlocks[0]->setText("The old lighthouse keeper, Elias, watched the churning sea from his perch. The wind howled a mournful tune, a sound he had grown to love over his fifty years on this isolated rock. Each wave that crashed against the cliffs below was a familiar roar, a testament to the raw power of nature. He was a man carved from the same stone as the lighthouse itself—weathered, silent, and standing firm against the relentless elements.\r\rHe remembered the first time he saw the light. He was a young boy, his fathers apprentice, and the beam had cut through the fog like a silver knife. It was a beacon of hope, a promise of safety in a world of darkness and uncertainty. Now, he was the keeper of that promise. The responsibility was a heavy cloak, but one he wore with quiet pride. He checked the pressure gauges, polished the lens until it gleamed, and made sure the clockwork mechanism was wound tight, its steady tick a comforting rhythm in the lonely tower.\r\rHis only companions were the gulls, who nested on the craggy ledges and filled the air with their cries. They were raucous and demanding, but their presence was a constant reminder that he wasnt truly alone. Hed often throw them scraps of fish from his meager meals, watching as they squabbled and dove with chaotic grace. They were wild, free creatures, a stark contrast to his own solitary, structured life.\r\rOne evening, a storm unlike any he had seen began to brew. The sky turned a bruised purple, and the waves rose into monstrous, frothing peaks. The wind shrieked with a fury that rattled the very foundations of the lighthouse. Elias, with a practiced calm, worked to keep the light burning bright. He knew the lives of sailors depended on that single, unwavering beam. He fought against the wind as he refueled the lantern, his hands steady despite the tempests rage. He was a small man against an immense force, but his will was stronger than the gale.\r\rHours later, as the storm finally began to abate, a small fishing trawler appeared on the horizon. Its mast was splintered, its sail in tatters. It was a ghost ship, limping toward the safety of the harbor. Elias watched, his heart pounding with a mix of relief and anxiety. He knew the captain, an old friend named Thomas. He had been out at sea for days, and the storm had caught him unaware. The lighthouses light had been his only guide, his last hope.\r\rThe next day, Thomas came to visit. He was bruised and exhausted, but his eyes were full of gratitude. Elias, he said, his voice hoarse, that light... it saved us. We were lost, and all I could see was your beacon. It was a miracle. Elias simply nodded, his face impassive. He didnt need thanks. The knowledge that he had done his duty, that he had helped his friend find his way home, was reward enough.\r\rHe continued his vigil, day after day, year after year. The world outside the lighthouse changed. Ships became bigger, technology advanced, and soon, his job became obsolete. The automated systems were more reliable, more efficient. The government sent a letter, informing him of his retirement. He was to be the last keeper of this light.\r\rThe thought of leaving the tower was like a physical ache. This was his home, his purpose. But he understood. The world moved on, and so must he. On his last night, he went through his final checks. He polished the lens one last time, his reflection a blurry image in the gleaming glass. He took one last look at the sea, its surface now calm and placid. It was the same sea that had been his constant companion, his only witness to a life of quiet dedication. As the new automated light system flickered to life, casting a cold, impersonal beam, Elias felt a strange sense of peace. His time was over, but the light, the promise, would go on. He was just a man, but for half a century, he had been the heart of the lighthouse, and the light in the dark. He left the tower, a shadow against the dawn, and walked toward a new, uncertain horizon, the memory of the lighthouses steady rhythm and the seas wild roar etched forever in his soul.");
    textBlocks[0]->update();
//    SelectedString::textBlock = textBlocks[0].get();
}

void Renderer::draw() {
    NS::AutoreleasePool* autoreleasePool = NS::AutoreleasePool::alloc()->init();
    
//    this->frameSemaphore.acquire();
    
    // try per quad rendering instead of this?
    
    MTL::CommandBuffer* commandBuffer = commandQueue->commandBuffer();
    MTL::RenderPassDescriptor* renderPassDescriptor = view->currentRenderPassDescriptor();
    MTL::RenderCommandEncoder* renderCommandEncoder = commandBuffer->renderCommandEncoder(renderPassDescriptor);
    renderCommandEncoder->setRenderPipelineState(this->renderPipelineState);

    for (auto& textBlock: textBlocks) {
        renderCommandEncoder->setVertexBuffer(textBlock->quadBuffer, 0, 0);
        
        renderCommandEncoder->setFragmentBuffer(textBlock->contoursBuffer, 0, 1);
        renderCommandEncoder->setFragmentBuffer(textBlock->contoursMetaBuffer, 0, 2);
        renderCommandEncoder->setFragmentBuffer(textBlock->uniformsBuffer, 0, 3);
        
        renderCommandEncoder->drawPrimitives(MTL::PrimitiveType::PrimitiveTypeTriangle, NS::UInteger(0), NS::UInteger(textBlock->numQuads*3*2));
    }
    
    renderCommandEncoder->endEncoding();
    
    std::function<void(MTL::CommandBuffer*)> completedHandler = [this](MTL::CommandBuffer* commandBuffer){
//        this->frameSemaphore.release();
    };
    
    commandBuffer->addCompletedHandler(completedHandler);
    commandBuffer->presentDrawable(view->currentDrawable());
    commandBuffer->commit();
    
    autoreleasePool->release();
}

Renderer::~Renderer() {
    commandQueue->release();
    renderPipelineState->release();
    FT_Done_FreeType(ft);
}
