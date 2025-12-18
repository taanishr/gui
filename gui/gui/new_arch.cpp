//
//  new_arch.cpp
//  gui
//
//  Created by Taanish Reja on 11/20/25.
//

#include "new_arch.hpp"

namespace NewArch {
    // pipeline specific
    UIContext::UIContext(MTL::Device* device, MTK::View* view):
        device{device},
        view{view},
        allocator{DrawableBufferAllocator{device}},
        layoutEngine{},
        frameInfoBuffer{allocator.allocate(sizeof(FrameInfo))}
    {
        auto frameDimensions = this->view->drawableSize();

        FrameInfo frameInfo {.width=static_cast<float>(frameDimensions.width)/2.0f, .height=static_cast<float>(frameDimensions.height)/2.0f};
        
        std::memcpy(frameInfoBuffer.get()->contents(), &frameInfo, sizeof(FrameInfo));
    };

    std::vector<simd_float2> LayoutEngine::resolve(Constraints& constraints, Atomized atomized)
    {
        float running_width = 0.0;
        float running_height = 0.0;

        std::vector<simd_float2> raw_placements;
    
        simd_float2 cursor {
            constraints.x,
            constraints.y
        };

        for (auto i = 0; i < atomized.atoms.size(); ++i) {
            auto& curr_atom = atomized.atoms[i];

            raw_placements.push_back(cursor);
            cursor.x += curr_atom.width;
        }
        
        return raw_placements;
    }
}
