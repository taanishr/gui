//
//  new_arch.cpp
//  gui
//
//  Created by Taanish Reja on 11/20/25.
//

#include "new_arch.hpp"
#include "fragment_types.hpp"
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


    // absolute needs:
    /*
        - Out-of-flow: does not affect siblings or the parent cursor.
        - Containing block selection:
            - nearest ancestor with position != static (i.e. positioned ancestor) -> containing block
            - otherwise -> root frame.
        - Position coordinates are relative to the containing block origin (top-left by default).
        - Internal layout (for its children) still uses inline/block semantics and the same
          line-box logic, but the "available width" is the containing block's inner width.
        - Shrink-to-fit for inline absolute boxes:
            - Compute minContentWidth and maxContentWidth for the node.
            - resolvedWidth = min( max(minContentWidth, availableWidth), maxContentWidth ).
            - If minContentWidth > availableWidth => inline internal layout degenerates to
              block-like stacking (vertical flow of line boxes).
        - Absolute elements must carry owner/style and be atomized as content atoms; any
          background/box rect for the absolute node is emitted in Finalize after layout.
    */

    // fixed and absolute, block/inline
    LayoutResult LayoutEngine::layoutAbsolute(Constraints& constraints, simd_float2 currentCursor, LayoutInput& layoutInput, Atomized& atomized) {
        LayoutResult lr;
        lr.outOfFlow = true;

        simd_float2 absolutePosition = constraints.origin;
        simd_float2 newCursor = absolutePosition;
        float resolvedHeight = 0.0f;
        float resolvedWidth = 0.0f;

        std::vector<simd_float2> atomOffsets;
        // blockify all elements
        for (auto& atom : atomized.atoms) {
            atomOffsets.push_back(newCursor);
            newCursor.x += atom.width;  
            resolvedWidth += atom.width;
            resolvedHeight = std::max(resolvedHeight, atom.height);
        }

        lr.atomOffsets = atomOffsets;
        lr.siblingCursor = currentCursor;
        lr.consumedHeight = 0;

        lr.childConstraints = {
            .origin = absolutePosition,
            .cursor = absolutePosition,
            .maxWidth = resolvedWidth,
            .maxHeight = resolvedHeight,
            .frameInfo = constraints.frameInfo
        };

        lr.computedBox = {
            .x = constraints.cursor.x,
            .y = constraints.cursor.y,
            .width = resolvedWidth,
            .height = resolvedHeight
        };

        return lr;
    }


    // fixed needs:
    /*
        - Out-of-flow: does not affect siblings or the parent cursor.
        - Containing block: the viewport (screen). Coordinates are relative to viewport origin.
        - Requires awareness of viewport dimensions (screenWidth/screenHeight) as available space.
        - Internal layout respects inline vs block semantics; shrink-to-fit applies the same way:
            - If minContentWidth > viewportAvailableWidth => degenerate to block-like stacking.
        - Position and final atom placement are independent of scrolling of any containing block.
    */
    LayoutResult LayoutEngine::layoutFixed(
        Constraints& constraints, 
        simd_float2 currentCursor, 
        LayoutInput& layoutInput, 
        Atomized& atomized
    ) {
        LayoutResult lr;
        lr.outOfFlow = true;

        simd_float2 viewportOrigin = {0.0f, 0.0f};
        simd_float2 fixedPosition = viewportOrigin;
        
        // TODO: Apply top/left/right/bottom offsets when added
        // if (layoutInput.hasLeft) fixedPosition.x += layoutInput.left;
        // if (layoutInput.hasTop) fixedPosition.y += layoutInput.top;
        
        simd_float2 newCursor = fixedPosition;
        float resolvedHeight = 0.0f;
        float resolvedWidth = 0.0f;
        std::vector<simd_float2> atomOffsets;

        for (auto& atom : atomized.atoms) {
            atomOffsets.push_back(newCursor);
            newCursor.x += atom.width;
            resolvedWidth += atom.width;
            resolvedHeight = std::max(resolvedHeight, atom.height);
        }

        lr.atomOffsets = atomOffsets;
        
        lr.siblingCursor = currentCursor;
        lr.consumedHeight = 0;

        lr.childConstraints = {
            .origin = viewportOrigin,
            .cursor = fixedPosition,                  
            .maxWidth = resolvedWidth,
            .maxHeight = resolvedHeight,
            .frameInfo = constraints.frameInfo
        };

        lr.computedBox = {
            .x = fixedPosition.x,
            .y = fixedPosition.y,
            .width = resolvedWidth,
            .height = resolvedHeight
        };

        return lr;
    }


    LayoutResult LayoutEngine::resolveOutOfFlow(Constraints& constraints, simd_float2 currentCursor, LayoutInput& layoutInput, Atomized& atomized){
        LayoutResult lr;

        if (layoutInput.position == Position::Absolute) {
            lr = layoutAbsolute(constraints, currentCursor, layoutInput, atomized);
        }else {
            lr =layoutFixed(constraints, currentCursor, layoutInput, atomized);
        }

        return lr;
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
        childConstraints.origin = currentCursor;
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
        
        lr.childConstraints.origin = currentCursor;
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
        LayoutResult lr;
        
       simd_float2 cursor = constraints.cursor;
       
    //    for (auto i = 0; i < atomized.atoms.size(); ++i) {
    //        auto& curr_atom = atomized.atoms[i];

    //        atomOffsets.push_back(cursor);
    //        cursor.x += curr_atom.width;
    //    }

        
        if (layoutInput.position == Position::Fixed || layoutInput.position == Position::Absolute) {
            lr = resolveOutOfFlow(constraints, constraints.cursor, layoutInput, atomized);
        }else {
            lr = resolveNormalFlow(constraints, constraints.cursor, layoutInput, atomized);
        }
        
        return lr;
    }
}
