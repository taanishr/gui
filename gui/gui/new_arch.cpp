//
//  new_arch.cpp
//  gui
//
//  Created by Taanish Reja on 11/20/25.
//

#include "new_arch.hpp"

namespace NewArch {
    UIContext::UIContext(MTL::Device* device):
        device{device}, allocator{DrawableBufferAllocator{device}}
    {};

    Shell::Shell(UIContext& ctx, float height, float width):
        ctx{ctx}, height{height}, width{width}
    {}

    std::vector<Atom> Shell::atomize() {
        std::vector<Atom> atoms {};
        
        // prepare buffer
        unsigned long bufferLen = 6*sizeof(simd_float2);
        auto bufferHandle = ctx.allocator.allocate(bufferLen);
        auto buf = ctx.allocator.get(bufferHandle);
        std::array<simd_float2, 6> shell_points {{
            {0,0},
            {width,0},
            {0,height},
            {0,height},
            {width,0},
            {width,height},
        }};
        std::memcpy(buf->contents(), shell_points.data(), shell_points.size() * sizeof(simd_float2));
        
        // finish allocating atom
        Atom atom;
        atom.bufferHandle = bufferHandle;
        atom.bufferOffset = 0;
        atom.length = bufferLen;
        atom.width = width;
        atom.height = height;
        
        atoms.push_back(atom);
        return atoms;
    }
}
