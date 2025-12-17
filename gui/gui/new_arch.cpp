//
//  new_arch.cpp
//  gui
//
//  Created by Taanish Reja on 11/20/25.
//

#include "new_arch.hpp"

namespace NewArch {
    UIContext::UIContext(MTL::Device* device, MTK::View* view):
        device{device}, view{view}, allocator{DrawableBufferAllocator{device}}
    {};

    void ShellUniforms::init_shape_dep(float width, float height) {
        this->halfExtent = {
            width / 2.0f,
            height / 2.0f
        };
    }

    void ShellUniforms::init_layout_dep(Layout& layout) {
        this->rectCenter = {
            layout.x + halfExtent.x,
            layout.y + halfExtent.y,
        };
    }

    std::vector<Atom> Shell::atomize() {
        std::vector<Atom> atoms {};
        
        // prepare buffer
        unsigned long bufferLen = 6*sizeof(AtomPoint);
        auto bufferHandle = ctx.allocator.allocate(bufferLen);
        auto buf = ctx.allocator.get(bufferHandle);
        
        std::array<AtomPoint, 6> shell_points {{
            {{0,0}, 0},
            {{width,0}, 0},
            {{0,height}, 0},
            {{0,height}, 0},
            {{width,0}, 0},
            {{width,height}, 0},
        }};
        
        std::memcpy(buf->contents(), shell_points.data(), shell_points.size() * sizeof(AtomPoint));
        
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

    LayoutEngine::LayoutEngine(UIContext& ctx):
        ctx{ctx}
    {}

    Encoder::Encoder(UIContext& ctx):
        ctx{ctx}
    {
        // set up frame info buffer
        auto frameDimensions = ctx.view->drawableSize();
        FrameInfo fi {.width=static_cast<float>(frameDimensions.width)/2.0f, .height=static_cast<float>(frameDimensions.height)/2.0f};
        this->frameInfo = ctx.allocator.allocate(sizeof(frameInfo));
        auto fiBuf = ctx.allocator.get(this->frameInfo);
        std::memcpy(fiBuf->contents(), &fi, sizeof(frameInfo));
    }
}
