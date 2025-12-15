//
//  newarch.hpp
//  gui
//
//  Created by Taanish Reja on 11/20/25.
//

#pragma once
#include "fragment_types.hpp"
#include "metal_imports.hpp"
#include "frame_info.hpp"
#include <ranges>

using namespace std::ranges::views;

class Renderer;

namespace NewArch {
    struct UIContext {
        UIContext(MTL::Device* device, MTK::View* view);
        
        MTL::Device* device;
        MTK::View* view;
        DrawableBufferAllocator allocator;
    };

    struct Cursor {
        simd_float2 position;
    };

    struct Layout {
        float x;
        float y;
    };

    struct ShellUniforms {
        // constant
        simd_float4 color;
        float cornerRadius;
        float borderWidth;
        simd_float4 borderColor;
        
        // layout dependent
        simd_float2 rectCenter;
        
        // shape dependent
        simd_float2 halfExtent;
        
        void init_shape_dep(float width, float height);
        
        void init_layout_dep(Layout& layout);
    };
    
    struct AtomPoint {
        simd_float2 point;
        unsigned int id;
    };

    struct Shell {
        Shell(UIContext& ctx, float width, float height);

        void buildPipeline(MTL::RenderPipelineState*& pipeline);
        MTL::RenderPipelineState* getPipeline();
        std::vector<Atom> atomize();
        
        float width;
        float height;
        UIContext& ctx;
    };

    struct LayoutEngine {
        LayoutEngine(UIContext& ctx);
        
        template <typename T>
        FragmentTemplate<T> place(
            Layout& layout,
            T uniforms_data,
            std::vector<Atom>& atoms
        )
        {
            Uniforms<T> uniforms;
            FragmentTemplate<T> fragment;
        
            // first copy atoms
            fragment.atoms = atoms;
            
            // build placements
            // simplcity first; handle actual cases later
            // TODO CASES:
            // block vs. inline
            // an actual globally-shared cursor
            // handling overflow on the screen
            // actual positioning (absolute vs. relative)
            
            auto placementBufferHandle = ctx.allocator.allocate(atoms.size()*sizeof(simd_float2));
            auto placementBuffer = ctx.allocator.get(placementBufferHandle);
            
            
            simd_float2 cursor {
                layout.x,
                layout.y
            };
            
            float running_width = 0.0;
            float running_height = 0.0;
            
            std::vector<simd_float2> raw_placements;
            std::vector<AtomPlacement> placements;
            
            for (auto i = 0; i < atoms.size(); ++i) {
                auto& curr_atom = atoms[i];
                AtomPlacement placement;
                placement.x = cursor.x;
                placement.y = cursor.y;
                placement.placementBufferOffset = i*sizeof(simd_float2);
                placement.placementBufferHandle = placementBufferHandle;
                
                raw_placements.push_back(cursor);
                placements.push_back(placement);
                
                running_width += curr_atom.width;
                running_height = std::max(running_height, curr_atom.height);
                cursor.x += curr_atom.width;
            }
            
            std::memcpy(placementBuffer->contents(), raw_placements.data(), raw_placements.size()*sizeof(simd_float2));
            
            fragment.atomPlacements = placements;
            
            // handle total width/height
            fragment.width = running_width;
            fragment.height = running_height;
            
            // copy uniforms
            auto uniformsBufferHandle = ctx.allocator.allocate(sizeof(T));
            auto uniformsBuffer = ctx.allocator.get(uniformsBufferHandle);
            std::memcpy(uniformsBuffer->contents(), &uniforms_data, sizeof(T));
            
            uniforms.uniformsBufferHandle = uniformsBufferHandle;
            uniforms.uniforms = uniforms_data;

            fragment.uniforms = uniforms;
            
            return fragment;
        }
        
        UIContext& ctx;
    };


    struct Encoder {
        Encoder(UIContext& ctx);
        
        template <typename T>
        void encode(MTL::RenderCommandEncoder* renderCommandEncoder, MTL::RenderPipelineState* pipeline, FragmentTemplate<T>& ft) {
            renderCommandEncoder->setRenderPipelineState(pipeline);
            
            for (auto [atom, atomPlacement] : zip(ft.atoms, ft.atomPlacements)) {
                // vertex buffers
                auto atomBuf = ctx.allocator.get(atom.bufferHandle);
                auto atomPlacementBuf = ctx.allocator.get(atomPlacement.placementBufferHandle);
                auto frameInfoBuf = ctx.allocator.get(this->frameInfo);
                
                // fragment buffers
                auto uniformsBuf = ctx.allocator.get(ft.uniforms.uniformsBufferHandle);
                
                renderCommandEncoder->setVertexBuffer(atomBuf, 0, 0);
                renderCommandEncoder->setVertexBuffer(atomPlacementBuf, 0, 1);
                renderCommandEncoder->setVertexBuffer(frameInfoBuf, 0, 2);

                renderCommandEncoder->setFragmentBuffer(uniformsBuf, 0, 0);
                
                std::println("atom.bufferOffset: {} atom.length: {}", atom.bufferOffset, atom.length /  sizeof(AtomPoint));
                
                // assert works fine on raw points?
//                std::array<simd_float2, 6> shell_points {{ {0,0}, {300,0}, {0,300}, {0,300}, {300,0}, {300,300} }};
//                assert(std::memcmp(atomBuf->contents(), shell_points.data(), shell_points.size() * sizeof(simd_float2)) == 0);
    
                // assert frames are same; asserts well
                auto frameDimensions = ctx.view->drawableSize();
                FrameInfo fi {.width=static_cast<float>(frameDimensions.width)/2.0f, .height=static_cast<float>(frameDimensions.height)/2.0f};
                assert(std::memcmp(frameInfoBuf->contents(), &fi, sizeof(FrameInfo)) == 0);
                
                // atom.length is wrong; buffer size, not num points
                renderCommandEncoder->drawPrimitives(MTL::PrimitiveTypeTriangle, atom.bufferOffset, atom.length / sizeof(AtomPoint));
            }
        }
        
        BufferHandle frameInfo;
        UIContext& ctx;
    };

}
