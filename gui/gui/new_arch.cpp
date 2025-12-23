//
//  new_arch.cpp
//  gui
//
//  Created by Taanish Reja on 11/20/25.
//

#include "new_arch.hpp"
#include <simd/vector_types.h>

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

    // fixed and absolute, block/inline
    void LayoutEngine::layoutAbsolute(Constraints& constraints, LayoutInput& layoutInput, Atomized& atomized) {
        
    }

    void LayoutEngine::layoutFixed(Constraints& constraints, LayoutInput& layoutInput, Atomized& atomized) {
        
    }

    void LayoutEngine::resolveOutOfFlow(Constraints& constraints, LayoutInput& layoutInput, Atomized& atomized){
        
    }

    // relative, block/inline
    LayoutResult LayoutEngine::layoutBlockNormalFlow(Constraints& constraints, simd_float2 currentCursor, LayoutInput& layoutInput, Atomized& atomized) {
        // TODO: reset cursor if to default position ??
        LayoutResult lr;
        std::vector<simd_float2> atomOffsets;
        Constraints childConstraints;
        simd_float2 newCursor {constraints.cursor.x, currentCursor.y};


        lr.outOfFlow = false;
        
        float resolvedWidth = 0;
        float resolvedHeight = 0;
        childConstraints.cursor = newCursor;
        childConstraints.frameInfo = constraints.frameInfo;
            
    
        for (auto& atom : atomized.atoms) {
            atomOffsets.push_back(newCursor);
            newCursor.x += atom.width;
            resolvedWidth += atom.width;
            resolvedHeight = std::max(resolvedHeight, atom.height);
        }

        // add resolved width check (not sure what behavior should be)

        lr.computedBox = {
            constraints.cursor.x,
            currentCursor.y,
            resolvedWidth,
            resolvedHeight
        };

        lr.consumedHeight = resolvedHeight;
                
        childConstraints.maxHeight = resolvedHeight;
        childConstraints.maxWidth = resolvedWidth;

        lr.childConstraints = childConstraints;
        lr.atomOffsets = atomOffsets;

        newCursor.y += resolvedHeight;
        newCursor.x = constraints.cursor.x;

        lr.siblingCursor = newCursor;
        

        return lr;
    }

    LayoutResult LayoutEngine::layoutInlineNormalFlow(
        Constraints& constraints, 
        simd_float2 currentCursor, 
        LayoutInput& layoutInput, 
        Atomized& atomized
    ) {
        LayoutResult lr;
        lr.outOfFlow = false;
        
        std::vector<simd_float2> atomOffsets;
        simd_float2 newCursor = currentCursor;
        
        float lineHeight = 0;
        float totalWidth = 0;
        float totalHeight = 0;
        float minX = currentCursor.x;
        float maxX = currentCursor.x;
        
        for (auto& atom : atomized.atoms) {
            if (newCursor.x + atom.width > constraints.cursor.x + constraints.maxWidth && 
                newCursor.x > constraints.cursor.x) {
                
                newCursor.x = constraints.cursor.x;
                newCursor.y += lineHeight;
                totalHeight += lineHeight;
                lineHeight = 0;
            }
            
            atomOffsets.push_back(newCursor);
            newCursor.x += atom.width;
            lineHeight = std::max(lineHeight, atom.height);
            
            minX = std::min(minX, newCursor.x - atom.width);
            maxX = std::max(maxX, newCursor.x);
        }
        
        totalHeight += lineHeight;
        totalWidth = maxX - minX;
        
        lr.computedBox = {
            minX,
            currentCursor.y,
            totalWidth,
            totalHeight
        };
        
        lr.childConstraints.cursor = {minX, currentCursor.y};
        lr.childConstraints.maxWidth = totalWidth;
        lr.childConstraints.maxHeight = totalHeight;
        lr.childConstraints.frameInfo = constraints.frameInfo;
        
        lr.atomOffsets = atomOffsets;
        lr.consumedHeight = totalHeight;
        
        if (newCursor.x >= constraints.cursor.x + constraints.maxWidth) {
            lr.siblingCursor = {constraints.cursor.x, newCursor.y + lineHeight};
        } else {
            lr.siblingCursor = {newCursor.x, newCursor.y};
        }
        
        return lr;
    }

    LayoutResult LayoutEngine::resolveNormalFlow(Constraints& constraints, simd_float2 current_cursor, LayoutInput& layoutInput, Atomized& atomized) {
        LayoutResult lr;

        if (layoutInput.display == Display::Block) {
            lr = layoutBlockNormalFlow(constraints, current_cursor, layoutInput, atomized);
        }else {
            lr =layoutInlineNormalFlow(constraints, current_cursor, layoutInput, atomized);
        }

        return lr;
    }

    LayoutResult LayoutEngine::resolve(Constraints& constraints, LayoutInput& layoutInput, Atomized atomized)
    {
        // LayoutResult lr;
        
//        simd_float2 cursor = constraints.cursor;
//        
//        for (auto i = 0; i < atomized.atoms.size(); ++i) {
//            auto& curr_atom = atomized.atoms[i];
//
//            atomOffsets.push_back(cursor);
//            cursor.x += curr_atom.width;
//        }

        
        // if (layoutInput.position == Position::Fixed || layoutInput.position == Position::Absolute) {
        //     // resolveOutOfFlow(constraints, cuconstraints.cursor, layoutInput, atomized);
        // }else {
        //     resolveNormalFlow(constraints, constraints.cursor, layoutInput, atomized);
        // }

        auto lr = resolveNormalFlow(constraints, constraints.cursor, layoutInput, atomized);
        
        return lr;
    }
}
