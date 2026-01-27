//
//  new_arch.cpp
//  gui
//
//  Created by Taanish Reja on 11/20/25.
//

#pragma once

#include "new_arch.hpp"
#include "fragment_types.hpp"
#include <algorithm>
#include <print>
#include <simd/vector_types.h>

namespace NewArch {

    // pipeline specific
    UIContext::UIContext(MTL::Device* device, MTK::View* view):
        device{device},
        view{view},
        allocator{DrawableBufferAllocator{device}},
        layoutEngine{},
        frameInfoBuffer{allocator.allocate(sizeof(FrameInfo))},
        frameIndex{0}
    {
        auto frameDimensions = this->view->drawableSize();
        auto scale = AppKit_Extensions::getContentScaleFactor(reinterpret_cast<void*>(view));

        FrameInfo frameInfo {.width=static_cast<float>(frameDimensions.width)/2.0f, .height=static_cast<float>(frameDimensions.height)/2.0f, .scale = scale};
        
        std::memcpy(frameInfoBuffer.get()->contents(), &frameInfo, sizeof(FrameInfo));
    };

    void UIContext::updateView() {
        auto frameDimensions = this->view->drawableSize();

        std::println("width: {} height: {}", frameDimensions.width, frameDimensions.width);

        auto scale = AppKit_Extensions::getContentScaleFactor(reinterpret_cast<void*>(view));

        FrameInfo frameInfo {.width=static_cast<float>(frameDimensions.width)/2.0f, .height=static_cast<float>(frameDimensions.height)/2.0f, .scale = scale};
        
        std::memcpy(frameInfoBuffer.get()->contents(), &frameInfo, sizeof(FrameInfo));
    }

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


        absolutePosition.x += layoutInput.left;
        absolutePosition.y += layoutInput.top;

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

        // std::println("resolvedWidth: {}", resolvedWidth - layoutInput.paddingLeft - layoutInput.paddingRight);

        lr.childConstraints = {
            .origin = {absolutePosition.x + layoutInput.paddingLeft, absolutePosition.y + layoutInput.paddingTop},
            .cursor = {absolutePosition.x + layoutInput.paddingLeft, absolutePosition.y + layoutInput.paddingTop},
            .maxWidth = resolvedWidth - layoutInput.paddingLeft - layoutInput.paddingRight,
            .maxHeight = resolvedHeight - layoutInput.paddingTop - layoutInput.paddingBottom,
            .frameInfo = constraints.frameInfo
        };

        // std::println("lr.childConstraints.origin.x: {} lr.childConstraints.origin.y: {}", (float)lr.childConstraints.origin.x, (float)lr.childConstraints.origin.y);

        lr.computedBox = {
            .x = absolutePosition.x,
            .y = absolutePosition.y,
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
    

        fixedPosition.x += layoutInput.left;
        fixedPosition.y += layoutInput.top;
        
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
            .cursor = {fixedPosition.x + layoutInput.paddingLeft, fixedPosition.y + layoutInput.paddingTop},                  
            .maxWidth = resolvedWidth - layoutInput.paddingLeft - layoutInput.paddingRight,
            .maxHeight = resolvedHeight - layoutInput.paddingTop - layoutInput.paddingBottom,
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

        float startingX = constraints.origin.x;
        float startingY = constraints.cursor.y;
        
        if (constraints.edgeIntent.edgeDisplayMode == Display::Block) {
            if (constraints.edgeIntent.collapsable) {
                startingY += std::max(constraints.edgeIntent.intent, layoutInput.marginTop);
            }else {
                startingY += constraints.edgeIntent.intent + layoutInput.marginTop;
            }
        }

        startingX += layoutInput.marginLeft;


        simd_float2 newCursor {startingX, startingY};


        lr.outOfFlow = false;
        
        float resolvedWidth = 0;
        float resolvedHeight = 0;
        childConstraints.cursor.x = startingX + layoutInput.paddingLeft;
        childConstraints.cursor.y = startingY + layoutInput.paddingTop;
        childConstraints.origin = childConstraints.cursor;
        childConstraints.frameInfo = constraints.frameInfo;
            
    
        for (auto& atom : atomized.atoms) {
            atomOffsets.push_back(newCursor);
            newCursor.x += atom.width;
            resolvedWidth += atom.width;
            resolvedHeight = std::max(resolvedHeight, atom.height);
        }

        // add resolved width check (not sure what behavior should be)

        lr.computedBox = {
            startingX,
            startingY,
            resolvedWidth,
            resolvedHeight
        };

        lr.consumedHeight = resolvedHeight;
                
        childConstraints.maxHeight = resolvedHeight - layoutInput.paddingTop - layoutInput.paddingBottom;
        childConstraints.maxWidth = resolvedWidth - layoutInput.paddingLeft - layoutInput.paddingRight;

        lr.childConstraints = childConstraints;
        lr.atomOffsets = atomOffsets;

        newCursor.y += resolvedHeight;
        newCursor.x = constraints.origin.x;

        lr.siblingCursor = newCursor;

        lr.edgeIntent = {
            .edgeDisplayMode = Display::Block,
            .intent = layoutInput.marginBottom,
            .collapsable = true,
        };
        
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
        // newCursor.x += layoutInput.marginLeft;
        if (constraints.edgeIntent.edgeDisplayMode == Block) {
            if (constraints.edgeIntent.collapsable) {
                newCursor.y += std::max(layoutInput.marginTop, constraints.edgeIntent.intent);
            }else {
                newCursor.y += layoutInput.marginTop + constraints.edgeIntent.intent;
            }

            newCursor.x += layoutInput.marginLeft;
        }else if (constraints.edgeIntent.edgeDisplayMode == Inline){
            if (constraints.edgeIntent.collapsable) {
                newCursor.x += std::max(layoutInput.marginLeft, constraints.edgeIntent.intent);
            }else {
                newCursor.x += layoutInput.marginLeft + constraints.edgeIntent.intent;
            }
        }

        lr.childConstraints.origin = currentCursor; // double check this
        float lineHeight = 0;
        float totalWidth = 0;
        float totalHeight = 0;
        float minX = currentCursor.x;
        float maxX = currentCursor.x;
        
        for (auto& atom : atomized.atoms) {
            // hmtl css only breaks on white space. I need to change this to preprocess line boxes? Idk
            if ((constraints.maxWidth > 0 && newCursor.x + atom.width > constraints.origin.x + constraints.maxWidth)
                || (newCursor.x + atom.width > constraints.frameInfo.width) || atom.placeOnNewLine) {
                newCursor.x = constraints.origin.x;
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
        
        lr.childConstraints.cursor = {minX + layoutInput.paddingLeft, currentCursor.y + layoutInput.paddingTop};
        lr.childConstraints.maxWidth = totalWidth - layoutInput.paddingLeft - layoutInput.paddingRight;
        lr.childConstraints.maxHeight = totalHeight - layoutInput.paddingTop - layoutInput.paddingBottom;
        lr.childConstraints.frameInfo = constraints.frameInfo;
        
        lr.atomOffsets = atomOffsets;
        lr.consumedHeight = totalHeight;
        
        if (newCursor.x >= constraints.origin.x + constraints.maxWidth) {
            lr.siblingCursor = {constraints.origin.x, newCursor.y + lineHeight};
        } else {
            lr.siblingCursor = {newCursor.x, newCursor.y};
        }

        lr.edgeIntent = {
            .edgeDisplayMode = Inline,
            .intent = layoutInput.marginRight,
            .collapsable = false,
        };

        return lr;
    }

    LayoutResult LayoutEngine::resolveNormalFlow(Constraints& constraints, simd_float2 current_cursor, LayoutInput& layoutInput, Atomized& atomized) {
        LayoutResult lr;

        if (layoutInput.display == Display::Block) {
            lr = layoutBlockNormalFlow(constraints, current_cursor, layoutInput, atomized);
        }else {
            lr = layoutInlineNormalFlow(constraints, current_cursor, layoutInput, atomized);
        }

        return lr;
    }

    LayoutResult LayoutEngine::resolve(Constraints& constraints, LayoutInput& layoutInput, Atomized atomized)
    {
        LayoutResult lr;
        
        if (layoutInput.position == Position::Fixed || layoutInput.position == Position::Absolute) {
            lr = resolveOutOfFlow(constraints, constraints.cursor, layoutInput, atomized);
        }else {
            lr = resolveNormalFlow(constraints, constraints.cursor, layoutInput, atomized);
        }
        
        return lr;
    }
}
