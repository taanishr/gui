//
//  newarch.hpp
//  gui
//
//  Created by Taanish Reja on 11/20/25.
//

#include "fragment_types.hpp"
#include "metal_imports.hpp"

class Renderer;

namespace NewArch {
    template <typename T>
    concept Drawable = requires(T t, MTL::RenderCommandEncoder* encoder, simd_float2 pt) {
        { t.getPipeline() } -> std::same_as<MTL::RenderPipelineState*>; // get pipeline
        
        { t.update() } -> std::same_as<void>; // update buffers
        { t.encode(encoder) } -> std::same_as<void>; // encode
        
        { t.atomize() } -> std::same_as<FragmentTemplate>; // atomize; useful for layouts

        { t.contains(pt) } -> std::same_as<bool>; // loop over the atoms?
    };


    // centralized buffer allocaiton
    using BufferHandle = unsigned int;

    struct DrawableBuffer {
        BufferHandle bufferId;
        MTL::Buffer* buffer;
        
        DrawableBuffer(MTL::Device* device, unsigned int bufferId, unsigned int size);
        ~DrawableBuffer();
    };

    struct DrawableBufferAllocator{
        BufferHandle nextId;
        std::unordered_map<BufferHandle, DrawableBuffer> buffers;
        MTL::Device* device;
        
        DrawableBufferAllocator(MTL::Device* device);
        
        BufferHandle allocate(unsigned int size);
        MTL::Buffer* get(BufferHandle handle);
    };

    // end of centralized buffer allocation;

    struct DrawableSize {
        float width;
        float height;
    };


    struct QuadPoint {
        simd_float2 position;
    };

    struct Uniforms {
        simd_float4 color;
        simd_float2 rectCenter;
        simd_float2 halfExtent;
        float cornerRadius;
        float borderWidth;
        simd_float4 borderColor;
    };

    struct Shell {
        Shell(Renderer& renderer, simd_float4 color={0,0,0,1}, float cornerRadius = 0.0);
        
        void buildPipeline(MTL::RenderPipelineState*& pipeline);
        MTL::RenderPipelineState* getPipeline();
        
        void update();
        void encode(MTL::RenderCommandEncoder* encoder);

        const DrawableSize& measure() const;
        const FragmentTemplate atomize() const;
        
        bool contains(simd_float2 point) const;
        ~Shell();
        
        Renderer& renderer;
        
        MTL::Buffer* quadPointsBuffer;
        MTL::Buffer* uniformsBuffer;
        MTL::Buffer* frameInfoBuffer;
        
        // properties
        simd_float4 color;
        simd_float4 borderColor;
        float cornerRadius = 0.0;
        float borderWidth = 0.0;
        
        DrawableSize intrinsicSize;
    };


}
