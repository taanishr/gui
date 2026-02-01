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

    // Resolve auto margins for centering
    LayoutEngine::ResolvedMargins LayoutEngine::resolveAutoMargins(
        const LayoutInput& li,
        float availableWidth,
        float contentWidth
    ) {
        ResolvedMargins margins;

        // Vertical margins: auto resolves to 0
        margins.top = li.marginTop.resolveOr(0.0f, 0.0f);
        margins.bottom = li.marginBottom.resolveOr(0.0f, 0.0f);

        // Horizontal margins: check for auto centering
        bool leftAuto = li.marginLeft.isAuto();
        bool rightAuto = li.marginRight.isAuto();

        if (leftAuto && rightAuto) {
            // Both auto: center horizontally
            float remainingSpace = availableWidth - contentWidth;
            if (remainingSpace > 0) {
                float autoMargin = remainingSpace / 2.0f;
                margins.left = autoMargin;
                margins.right = autoMargin;
            } else {
                // No space: auto resolves to 0
                margins.left = 0.0f;
                margins.right = 0.0f;
            }
        } else if (leftAuto) {
            // Only left auto: push to right (absorb remaining space)
            margins.right = li.marginRight.resolveOr(availableWidth, 0.0f);
            float remainingSpace = availableWidth - contentWidth - margins.right;
            margins.left = std::max(0.0f, remainingSpace);
        } else if (rightAuto) {
            // Only right auto: resolves to 0 (default left alignment)
            margins.left = li.marginLeft.resolveOr(availableWidth, 0.0f);
            margins.right = 0.0f;
        } else {
            // Neither auto: resolve normally
            margins.left = li.marginLeft.resolveOr(availableWidth, 0.0f);
            margins.right = li.marginRight.resolveOr(availableWidth, 0.0f);
        }

        return margins;
    }

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

        // Resolve margins (Auto resolves to 0 for absolute positioning, centering handled separately)
        float marginLeft = layoutInput.marginLeft.resolveOr(constraints.maxWidth, 0.0f);
        float marginTop = layoutInput.marginTop.resolveOr(constraints.maxHeight, 0.0f);

        absolutePosition.x += layoutInput.left + marginLeft;
        absolutePosition.y += layoutInput.top + marginTop;

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

        // Resolve margins (Auto resolves to 0 for fixed positioning)
        float marginLeft = layoutInput.marginLeft.resolveOr(constraints.frameInfo.width, 0.0f);
        float marginTop = layoutInput.marginTop.resolveOr(constraints.frameInfo.height, 0.0f);

        fixedPosition.x += layoutInput.left + marginLeft;
        fixedPosition.y += layoutInput.top + marginTop;
        
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
        LayoutResult lr;
        std::vector<simd_float2> atomOffsets;
        Constraints childConstraints;

        // First compute content dimensions from atoms
        float contentWidth = 0;
        float contentHeight = 0;
        for (auto& atom : atomized.atoms) {
            contentWidth += atom.width;
            contentHeight = std::max(contentHeight, atom.height);
        }

        // If explicit width specified, use that for margin calculation
        if (layoutInput.width > 0) {
            contentWidth = layoutInput.width;
        }

        // Resolve auto margins for potential centering
        auto margins = resolveAutoMargins(layoutInput, constraints.maxWidth, contentWidth);

        float startingX = constraints.origin.x;
        float startingY = constraints.cursor.y;

        // Handle vertical margin collapse
        if (constraints.edgeIntent.edgeDisplayMode == Display::Block) {
            if (constraints.edgeIntent.collapsable && !layoutInput.marginTop.isAuto()) {
                startingY += std::max(constraints.edgeIntent.intent, margins.top);
            } else {
                startingY += constraints.edgeIntent.intent + margins.top;
            }
        }

        // Apply left margin (now potentially auto-computed for centering)
        startingX += margins.left;

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

        // Auto margins don't participate in margin collapse
        lr.edgeIntent = {
            .edgeDisplayMode = Display::Block,
            .intent = margins.bottom,
            .collapsable = !layoutInput.marginBottom.isAuto(),
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

        // For inline elements, auto margins resolve to 0
        float marginTop = layoutInput.marginTop.resolveOr(0.0f, 0.0f);
        float marginRight = layoutInput.marginRight.resolveOr(0.0f, 0.0f);
        float marginBottom = layoutInput.marginBottom.resolveOr(0.0f, 0.0f);
        float marginLeft = layoutInput.marginLeft.resolveOr(0.0f, 0.0f);

        std::vector<simd_float2> atomOffsets;
        simd_float2 newCursor = currentCursor;

        lr.childConstraints.origin = currentCursor;
        float lineHeight = 0;
        float totalHeight = 0;
        float minX = currentCursor.x;
        float maxX = currentCursor.x;

        size_t atomIndex = 0;

        if (constraints.lineboxes.empty()) {
            if (constraints.edgeIntent.edgeDisplayMode == Block) {
                if (constraints.edgeIntent.collapsable) {
                    newCursor.y += std::max(marginTop, constraints.edgeIntent.intent);
                } else {
                    newCursor.y += marginTop + constraints.edgeIntent.intent;
                }
                newCursor.x = constraints.origin.x + marginLeft;
            } else {
                if (constraints.edgeIntent.collapsable) {
                    newCursor.x += std::max(marginLeft, constraints.edgeIntent.intent);
                } else {
                    newCursor.x += marginLeft + constraints.edgeIntent.intent;
                }
            }

            for (auto& atom : atomized.atoms) {
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
                minX = std::min(minX, atomOffsets.back().x);
                maxX = std::max(maxX, newCursor.x);
            }
        } else {
            for (size_t lineIdx = 0; lineIdx < constraints.lineboxes.size(); ++lineIdx) {
                const Line& line = constraints.lineboxes[lineIdx];

                if (lineIdx == 0) {
                    if (constraints.edgeIntent.edgeDisplayMode == Inline) {
                        if (line.collapsable) {
                            if (constraints.edgeIntent.collapsable) {
                                newCursor.x += std::max(marginLeft, constraints.edgeIntent.intent);
                            } else {
                                newCursor.x += marginLeft + constraints.edgeIntent.intent;
                            }
                        } else {
                            if (newCursor.x + line.width > constraints.origin.x + constraints.maxWidth) {
                                newCursor.x = constraints.origin.x + marginLeft;
                                newCursor.y += lineHeight;
                                totalHeight += lineHeight;
                                lineHeight = 0;
                            } else {
                                if (constraints.edgeIntent.collapsable) {
                                    newCursor.x += std::max(marginLeft, constraints.edgeIntent.intent);
                                } else {
                                    newCursor.x += marginLeft + constraints.edgeIntent.intent;
                                }
                            }
                        }
                    } else {
                        if (constraints.edgeIntent.collapsable) {
                            newCursor.y += std::max(marginTop, constraints.edgeIntent.intent);
                        } else {
                            newCursor.y += marginTop + constraints.edgeIntent.intent;
                        }
                        newCursor.x = constraints.origin.x + marginLeft;
                    }
                } else {
                    if (line.collapsable) {
                    } else {
                        if (newCursor.x + line.width > constraints.origin.x + constraints.maxWidth) {
                            newCursor.x = constraints.origin.x;
                            newCursor.y += lineHeight;
                            totalHeight += lineHeight;
                            lineHeight = 0;
                        }
                    }
                }

                for (size_t i = 0; i < line.atomCount && atomIndex < atomized.atoms.size(); ++i, ++atomIndex) {
                    auto& atom = atomized.atoms[atomIndex];

                    if (atom.placeOnNewLine) {
                        newCursor.x = constraints.origin.x;
                        newCursor.y += lineHeight;
                        totalHeight += lineHeight;
                        lineHeight = 0;
                    }

                    atomOffsets.push_back(newCursor);
                    newCursor.x += atom.width;
                    lineHeight = std::max(lineHeight, atom.height);
                    minX = std::min(minX, atomOffsets.back().x);
                    maxX = std::max(maxX, newCursor.x);
                }
            }
        }

        totalHeight += lineHeight;
        float totalWidth = maxX - minX;

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

        lr.siblingCursor = newCursor;

        lr.edgeIntent = {
            .edgeDisplayMode = Inline,
            .intent = marginRight,
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
