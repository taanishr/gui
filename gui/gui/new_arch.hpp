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
    struct UIContext {
        UIContext(MTL::Device* device);
        
        MTL::Device* device;
        DrawableBufferAllocator allocator;
    };

    struct ShellUniforms {
        simd_float4 color;
        simd_float2 rectCenter;
        simd_float2 halfExtent;
        float cornerRadius;
        float borderWidth;
        simd_float4 borderColor;
    };
    
    struct Shell {
        Shell(UIContext& ctx, float width, float height);

        std::vector<Atom> atomize();
        
        float width;
        float height;
        UIContext& ctx;
    };

    struct Cursor {
        simd_float2 position;
    };

    template <typename T>
    struct Layout {
        float x;
        float y;
        T uniforms;
    };

    struct LayoutEngine {
        LayoutEngine(UIContext& ctx);
        
        template <typename T>
        FragmentTemplate<T> place(
            Layout<T>& layout,
            std::vector<Atom>& atoms
        )
        {
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
                .x = layout.x,
                .y = layout.y
            };
            
            float running_width;
            float running_height;
            
            std::vector<simd_float2> raw_placements;
            std::vector<AtomPlacement> placements;
            
            for (auto i = 0; i < atoms.size(); ++i) {
                auto& curr_atom = atoms[i];
                
                AtomPlacement placement;
                placement.x = cursor.x;
                placement.y = cursor.y;
                placement.offset = i*sizeof(simd_float2);
                placement.placementBufferHandle = placementBufferHandle;
                
                raw_placements.push_back(cursor);
                
                running_width += curr_atom.width;
                running_height = std::max(running_height, curr_atom.height);
                cursor.x += curr_atom.width;
            }
            
            std::memcpy(placementBuffer->contents(), raw_placements.data(), raw_placements.size()*sizeof(simd_float2));
            
            // handle total width/height
            fragment.width = running_width;
            fragment.height = running_height;
            
            // copy uniforms
            fragment.uniforms = layout.uniforms;
            
            return fragment;
        }
        
        UIContext& ctx;
    };
}
