//
//  newarch.hpp
//  gui
//
//  Created by Taanish Reja on 11/20/25.
//

#pragma once
#include "fragment_types.hpp"
#include "printers.hpp"
#include "metal_imports.hpp"
#include "frame_info.hpp"
#include <ranges>
#include "MTKTexture_loader.hpp"
#include <format>

using namespace std::ranges::views;

class Renderer;

namespace NewArch {
    using FragmentID = uint64_t;

    struct Constraints {
        float maxWidth;
        float maxHeight;
        float x;
        float y;
    };

    struct Measured {
        FragmentID id;
        float explicitWidth;
        float explicitHeight;
    };

    struct Atomized {
        FragmentID id;
        std::vector<Atom> atoms;
    };

    struct Placed {
        FragmentID id;
        std::vector<AtomPlacement> placements;
    };

    template <typename U>
    struct Finalized {
        FragmentID id;
        Atomized atomized;
        Placed placed;
        U uniforms;
    };

    struct LayoutEngine {
        std::vector<simd_float2> resolve(Constraints& constraints, Atomized atomized);
    };


    struct UIContext {
        UIContext(MTL::Device* device, MTK::View* view);

        MTL::Device* device;
        MTK::View* view;
        DrawableBufferAllocator allocator;
        LayoutEngine layoutEngine;
        DrawableBuffer frameInfoBuffer;
    };

    template <typename S>
    struct Fragment {
        Fragment(UIContext& ctx):
            id{Fragment::nextId++},
            fragmentStorage{ctx}
        {}
        
        static FragmentID nextId;
        FragmentID id;
        S fragmentStorage;
    };

    template <typename S>
    FragmentID Fragment<S>::nextId = 0;
}
