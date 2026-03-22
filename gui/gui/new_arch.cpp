//
//  new_arch.cpp
//  gui
//
//  Created by Taanish Reja on 11/20/25.
//

#pragma once

#include "new_arch.hpp"
#include "fragment_types.hpp"
#include "sizing.hpp"
#include <algorithm>
#include <optional>
#include <print>
#include <simd/vector_types.h>


/*
    figuring out how to introduce right justificaiton with rtl setting
        op 1) helper function that resolves the next cursor
            problems:
                - <span> dasdasd </span> <span>dasdasdas </span>
                - will combine; but in right justification case, cursor needs to be adjusted in this case
                - need some sort of "lookahead" function or preprocessing maybe?
                - there should be a better method though
                - inlineCtx {
                    currentLineW
                }

        Solution:
            - create concept of line boxes
            - currently, i have created "inline boxes"
            - but line boxes will be shared across all children
            - each line box will be assigned to a line box
            - clear this vector if the array is blank
            - preprocess these
*/

namespace NewArch {

    simd_float2 resolvePosition(const PositionResolutionContext& ctx) {
        simd_float2 resolvedPosition = ctx.currentCursor;

        switch (ctx.layoutInput.position) {
            case NewArch::Position::Fixed: {
                float refWidth = ctx.constraints.frameInfo.width;
                float refHeight = ctx.constraints.frameInfo.height;
                simd_float2 refOrigin = {0.0f, 0.0f};

                std::optional<float> left = ctx.layoutInput.left.has_value()
                    ? ctx.layoutInput.left->resolve(refWidth) : std::nullopt;
                std::optional<float> top = ctx.layoutInput.top.has_value()
                    ? ctx.layoutInput.top->resolve(refHeight) : std::nullopt;
                std::optional<float> right = ctx.layoutInput.right.has_value()
                    ? ctx.layoutInput.right->resolve(refWidth) : std::nullopt;
                std::optional<float> bottom = ctx.layoutInput.bottom.has_value()
                    ? ctx.layoutInput.bottom->resolve(refHeight) : std::nullopt;

                resolvedPosition = refOrigin;

                if (ctx.constraints.inheritedProperties.direction == Direction::ltr) {
                    if (left.has_value()) {
                        resolvedPosition.x = refOrigin.x + *left + ctx.margins.left;
                    } else if (right.has_value()) {
                        resolvedPosition.x = refOrigin.x + refWidth - ctx.margins.right - ctx.layoutInput.width - *right;
                    }
                } else {
                    if (right.has_value()) {
                        resolvedPosition.x = refOrigin.x + refWidth - ctx.margins.right - ctx.layoutInput.width - *right;
                    } else if (left.has_value()) {
                        resolvedPosition.x = refOrigin.x + *left + ctx.margins.left;
                    }
                }

                if (top.has_value()) {
                    resolvedPosition.y = refOrigin.y + *top + ctx.margins.top;
                } else if (bottom.has_value()) {
                    resolvedPosition.y = refOrigin.y + refHeight - ctx.layoutInput.height - ctx.margins.bottom - *bottom;
                }

                break;
            }
            case NewArch::Position::Absolute: {
                auto& cb = ctx.constraints.absoluteContainingBlock;
                float refWidth = cb.width;
                float refHeight = cb.height;
                simd_float2 refOrigin = cb.origin;

                std::optional<float> left = ctx.layoutInput.left.has_value()
                    ? ctx.layoutInput.left->resolve(refWidth) : std::nullopt;
                std::optional<float> top = ctx.layoutInput.top.has_value()
                    ? ctx.layoutInput.top->resolve(refHeight) : std::nullopt;
                std::optional<float> right = ctx.layoutInput.right.has_value()
                    ? ctx.layoutInput.right->resolve(refWidth) : std::nullopt;
                std::optional<float> bottom = ctx.layoutInput.bottom.has_value()
                    ? ctx.layoutInput.bottom->resolve(refHeight) : std::nullopt;

                resolvedPosition = refOrigin;

                if (ctx.constraints.inheritedProperties.direction == Direction::ltr) {
                    if (left.has_value()) {
                        resolvedPosition.x = refOrigin.x + *left + ctx.margins.left;
                    } else if (right.has_value()) {
                        resolvedPosition.x = refOrigin.x + refWidth - ctx.margins.right - ctx.layoutInput.width - *right;
                    }
                } else {
                    if (right.has_value()) {
                        resolvedPosition.x = refOrigin.x + refWidth - ctx.margins.right - ctx.layoutInput.width - *right;
                    } else if (left.has_value()) {
                        resolvedPosition.x = refOrigin.x + *left + ctx.margins.left;
                    }
                }

                if (top.has_value()) {
                    resolvedPosition.y = refOrigin.y + *top + ctx.margins.top;
                } else if (bottom.has_value()) {
                    resolvedPosition.y = refOrigin.y + refHeight - ctx.layoutInput.height - ctx.margins.bottom - *bottom;
                }

                break;
            }   
            case NewArch::Position::Relative:
            case NewArch::Position::Static: {
                switch (ctx.layoutInput.display) {
                    case NewArch::Display::Block: {
                        float startingX = ctx.constraints.origin.x;
                        float startingY = ctx.constraints.cursor.y;

                        if (ctx.constraints.edgeIntent.edgeDisplayMode == Display::Block) {
                            if (ctx.constraints.edgeIntent.collapsable && !ctx.layoutInput.marginTop.isAuto()) {
                                startingY += std::max(ctx.constraints.edgeIntent.intent, ctx.margins.top);
                            } else {
                                startingY += ctx.constraints.edgeIntent.intent + ctx.margins.top;
                            }
                        }

                        if (ctx.constraints.inheritedProperties.direction == Direction::ltr) {
                            startingX += ctx.margins.left;
                        } else {
                            startingX = ctx.constraints.origin.x + ctx.constraints.maxWidth - ctx.layoutInput.width - ctx.margins.right;
                        }

                        resolvedPosition = {startingX, startingY};
                        break;
                    }
                    case NewArch::Display::Inline: {
                        resolvedPosition = ctx.currentCursor;
                        break;
                    }
                    default:
                        break;
                }

                // Relative: apply offsets after computing static position
                if (ctx.layoutInput.position == Position::Relative) {
                    if (ctx.layoutInput.top.has_value()) {
                        resolvedPosition.y += ctx.layoutInput.top->resolveOr(ctx.constraints.maxHeight, 0.0f);
                    } else if (ctx.layoutInput.bottom.has_value()) {
                        resolvedPosition.y -= ctx.layoutInput.bottom->resolveOr(ctx.constraints.maxHeight, 0.0f);
                    }

                    if (ctx.layoutInput.left.has_value()) {
                        resolvedPosition.x += ctx.layoutInput.left->resolveOr(ctx.constraints.maxWidth, 0.0f);
                    } else if (ctx.layoutInput.right.has_value()) {
                        resolvedPosition.x -= ctx.layoutInput.right->resolveOr(ctx.constraints.maxWidth, 0.0f);
                    }
                }

                break;
            }
            default:
                break;
        }

        return resolvedPosition;
    }

    simd_float2 resolveSize(const SizeResolutionContext& ctx)
    {
        simd_float2 explicitSize {0,0};

        switch (ctx.position) {
            case NewArch::Position::Absolute:
            case NewArch::Position::Fixed: {
                if (ctx.requestedHeight.has_value()) {
                    explicitSize.y = ctx.requestedHeight->resolveOr(ctx.availableHeight, 0.0f);
                }else {
                    std::optional<float> resolvedTop;
                    std::optional<float> resolvedBottom;

                    if (ctx.top.has_value()) {
                        resolvedTop = ctx.top->resolveOr(ctx.availableHeight, 0.0f);
                    }

                    if (ctx.bottom.has_value()) {
                        resolvedBottom = ctx.bottom->resolveOr(ctx.availableHeight, 0.0f);
                    }

                    if (resolvedTop.has_value() && resolvedBottom.has_value()) {
                        explicitSize.y = ctx.availableHeight - *resolvedTop - *resolvedBottom;
                    }
                }

                if (ctx.requestedWidth.has_value()) {
                    explicitSize.x = ctx.requestedWidth->resolveOr(ctx.availableWidth, 0.0);
                }else {
                    std::optional<float> resolvedRight;
                    std::optional<float> resolvedLeft;

                    if (ctx.right.has_value()) {
                        resolvedRight = ctx.right->resolveOr(ctx.availableWidth, 0.0f);
                    }

                    if (ctx.left.has_value()) {
                        resolvedLeft = ctx.left->resolveOr(ctx.availableWidth, 0.0f);
                    }

                    if (resolvedRight.has_value() && resolvedLeft.has_value()) {
                        explicitSize.x = ctx.availableWidth - *resolvedRight - * resolvedLeft;
                    }
                }

                break;
            }
            default: {
                // static fallthrough 
                if (ctx.requestedHeight.has_value()) {
                    explicitSize.y = ctx.requestedHeight->resolveOr(ctx.availableHeight, 0.0f);
                }else {
                    explicitSize.y = 0;
                }

                if (ctx.requestedWidth) {
                    explicitSize.x = ctx.requestedWidth->resolveOr(ctx.availableWidth, 0.0f);
                }else {
                    explicitSize.x = 0;
                }
            }
        };

        return explicitSize;
    }

    // Resolve auto margins for centering
    ResolvedMargins LayoutEngine::resolveAutoMargins(
        const LayoutInput& li,
        const ReplacedAttributes& replacedAttributes,
        float availableWidth,
        float contentWidth
    ) {
        ResolvedMargins margins;

        // Vertical margins: auto resolves to 0
        margins.bottom = li.marginBottom.resolveOr(0.0f, 0.0f);

        if (replacedAttributes.marginTop.has_value()) {
            auto& replacedMarginTop = *replacedAttributes.marginTop;
            margins.top = replacedMarginTop.resolveOr(0.0f, 0.0f);
        }
        else {
            margins.top = li.marginTop.resolveOr(0.0f, 0.0f);
        }

        if (replacedAttributes.marginBottom.has_value()) {
            auto replacedMarginBottom = *replacedAttributes.marginBottom;
            margins.bottom = replacedMarginBottom.resolveOr(0.0f, 0.0f);
        }
        else {
            margins.bottom = li.marginBottom.resolveOr(0.0f, 0.0f);
        }

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

    ResolvedMargins resolveMargins(
        const LayoutInput& li
    ) {

        float marginTop = li.marginTop.resolveOr(0.0f, 0.0f);
        float marginRight = li.marginRight.resolveOr(0.0f, 0.0f);
        float marginBottom = li.marginBottom.resolveOr(0.0f, 0.0f);
        float marginLeft = li.marginLeft.resolveOr(0.0f, 0.0f);

        ResolvedMargins resolvedMargins {
            .top = marginTop,
            .right = marginRight,
            .bottom = marginBottom,
            .left = marginLeft,
        };

        return resolvedMargins;
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

        auto margins = constraints.resolvedMargins;

        PositionResolutionContext pctx {
            .currentCursor = currentCursor,
            .constraints = constraints,
            .layoutInput = layoutInput,
            .margins = margins
        };

        simd_float2 absolutePosition = resolvePosition(pctx);
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

        float paddingLeft = layoutInput.paddingLeft.resolveOr(constraints.maxWidth);
        float paddingTop = layoutInput.paddingTop.resolveOr(constraints.maxHeight);
        float paddingRight = layoutInput.paddingRight.resolveOr(constraints.maxWidth);
        float paddingBottom = layoutInput.paddingBottom.resolveOr(constraints.maxHeight);

        lr.childConstraints = {
            .origin = {absolutePosition.x + paddingLeft, absolutePosition.y + paddingTop},
            .cursor = {absolutePosition.x + paddingLeft, absolutePosition.y + paddingTop},
            .maxWidth = resolvedWidth - paddingLeft - paddingRight,
            .maxHeight = resolvedHeight - paddingTop - paddingBottom,
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

        auto margins = constraints.resolvedMargins;

        PositionResolutionContext pctx {
            .currentCursor = currentCursor,
            .constraints = constraints,
            .layoutInput = layoutInput,
            .margins = margins
        };

        simd_float2 fixedPosition = resolvePosition(pctx);
        
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

        float paddingLeft = layoutInput.paddingLeft.resolveOr(constraints.maxWidth);
        float paddingTop = layoutInput.paddingTop.resolveOr(constraints.maxHeight);
        float paddingRight = layoutInput.paddingRight.resolveOr(constraints.maxWidth);
        float paddingBottom = layoutInput.paddingBottom.resolveOr(constraints.maxHeight);

        lr.childConstraints = {
            .origin = viewportOrigin,
            .cursor = {fixedPosition.x + paddingLeft, fixedPosition.y + paddingTop},                  
            .maxWidth = resolvedWidth - paddingLeft - paddingRight,
            .maxHeight = resolvedHeight - paddingTop - paddingBottom,
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

        auto margins = constraints.resolvedMargins;

        PositionResolutionContext pctx {
            .currentCursor = currentCursor,
            .constraints = constraints,
            .layoutInput = layoutInput,
            .margins = margins
        };
        simd_float2 startingPos = resolvePosition(pctx);

        simd_float2 newCursor {startingPos};

        lr.outOfFlow = false;

        float resolvedWidth = 0;
        float resolvedHeight = 0;

        float paddingLeft = layoutInput.paddingLeft.resolveOr(constraints.maxWidth);
        float paddingTop = layoutInput.paddingTop.resolveOr(constraints.maxHeight);
        float paddingRight = layoutInput.paddingRight.resolveOr(constraints.maxWidth);
        float paddingBottom = layoutInput.paddingBottom.resolveOr(constraints.maxHeight);

        childConstraints.cursor.x = startingPos.x + paddingLeft;
        childConstraints.cursor.y = startingPos.y + paddingTop;
        childConstraints.origin = childConstraints.cursor;
        childConstraints.frameInfo = constraints.frameInfo;

        for (auto& atom : atomized.atoms) {
            atomOffsets.push_back(newCursor);
            newCursor.x += atom.width;
            resolvedWidth += atom.width;
            resolvedHeight = std::max(resolvedHeight, atom.height);
        }

        lr.computedBox = {
            startingPos.x,
            startingPos.y,
            resolvedWidth,
            resolvedHeight
        };

        lr.consumedHeight = resolvedHeight;

        childConstraints.maxHeight = resolvedHeight - paddingTop - paddingBottom;
        childConstraints.maxWidth = resolvedWidth - paddingLeft - paddingRight;

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

    void LineBox::pushFragment(const LineFragment& fragment) {
        fragmentOffsets.push_back(currentFragmentOffset);
        currentFragmentOffset += fragment.width;
        width += fragment.width;
        fragmentCount += 1;
    }

    // LayoutResult LayoutEngine::layoutFlex

    LayoutResult LayoutEngine::layoutInlineNormalFlow(
        Constraints& constraints,
        simd_float2 currentCursor,
        LayoutInput& layoutInput,
        Atomized& atomized
    ) {
        LayoutResult lr;
        lr.outOfFlow = false;

        ResolvedMargins margins = constraints.resolvedMargins;

        std::vector<simd_float2> atomOffsets;

        PositionResolutionContext pctx {
            .currentCursor = currentCursor,
            .constraints = constraints,
            .layoutInput = layoutInput,
            .margins = margins
        };

        simd_float2 newCursor = resolvePosition(pctx);

        lr.childConstraints.origin = newCursor;
        float lineHeight = 0;
        float totalHeight = 0;
        float totalWidth = 0;
        float minX = newCursor.x;
        float minY = newCursor.y;

        size_t atomIndex = 0;
        bool isLtr = constraints.inheritedProperties.direction == Direction::ltr;
        size_t prevLineBoxIndex = -1;

        size_t fragmentIdx = 0;
        for (auto it = constraints.lineFragments.begin(); it != constraints.lineFragments.end(); ++it, ++fragmentIdx) {
            const LineFragment& fragment = *it;

            auto& lineBox = constraints.lineBoxes[fragment.lineBoxIndex];
            float offset = lineBox.fragmentOffsets[fragment.fragmentIndex];
            float startingX = isLtr
                                ? constraints.origin.x + offset
                                : constraints.origin.x + constraints.maxWidth - lineBox.width + offset;


            newCursor.x = startingX;

            if (fragmentIdx == 0) {
                float inlineMargin = isLtr ? margins.right : margins.left;

                if (constraints.edgeIntent.edgeDisplayMode == Inline) {
                    if (constraints.edgeIntent.collapsable) {
                        float collapsed = std::max(inlineMargin, constraints.edgeIntent.intent);
                        newCursor.x += collapsed;
                    } else {
                        float total = inlineMargin + constraints.edgeIntent.intent;
                        newCursor.x += total;
                    }
                } else {
                    if (constraints.edgeIntent.collapsable) {
                        newCursor.y += std::max(margins.top, constraints.edgeIntent.intent);
                    } else {
                        newCursor.y += margins.top + constraints.edgeIntent.intent;
                    }

                    newCursor.x += (isLtr ? inlineMargin : -inlineMargin);
                }
            }

            if (fragment.lineBoxIndex != prevLineBoxIndex && prevLineBoxIndex != (size_t)-1) {
                newCursor.y += lineHeight;
                totalHeight += lineHeight;
                lineHeight = 0;
                newCursor.x = startingX;
            }


            for (size_t i = 0; i < fragment.atomCount && atomIndex < atomized.atoms.size(); ++i, ++atomIndex) {
                auto& atom = atomized.atoms[atomIndex];

                atomOffsets.push_back(newCursor);
                newCursor.x += atom.width;
                lineHeight = std::max(lineHeight, atom.height);
            }

            prevLineBoxIndex = fragment.lineBoxIndex;
            totalWidth += fragment.width;
        }

        // Add trailing margin for sibling cursor
        newCursor.x += isLtr ? margins.right : -margins.left;

        totalHeight += lineHeight;

        lr.computedBox = {
            minX,
            minY,
            totalWidth,
            totalHeight
        };

        auto cbx = minX;
        auto cby = minY;

        float paddingLeft = layoutInput.paddingLeft.resolveOr(constraints.maxWidth);
        float paddingTop = layoutInput.paddingTop.resolveOr(constraints.maxHeight);
        float paddingRight = layoutInput.paddingRight.resolveOr(constraints.maxWidth);
        float paddingBottom = layoutInput.paddingBottom.resolveOr(constraints.maxHeight);

        lr.childConstraints.cursor = {minX + paddingLeft, minY + paddingTop};
        lr.childConstraints.maxWidth = totalWidth - paddingLeft - paddingRight;
        lr.childConstraints.maxHeight = totalHeight - paddingTop - paddingBottom;
        lr.childConstraints.frameInfo = constraints.frameInfo;

        lr.atomOffsets = atomOffsets;
        lr.consumedHeight = totalHeight;

        lr.siblingCursor = newCursor;

        lr.edgeIntent = {
            .edgeDisplayMode = Inline,
            .intent = isLtr ? margins.right : margins.left,
            .collapsable = false,
        };

        return lr;
    }

    // LayoutResult layoutFlex(Constraints& constraints, simd_float2 currentCursor, LayoutInput& layoutInput, Atomized& atomized) {
    //     // flex algorithm:
    //         // all in preprocessing: gather sizes (flex-basis) of all elements
    //         // if shrink is set: shrink elements; if grow is set: grow elements
    //         // place sequentially; leave remaining space

    // }


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

        if (layoutInput.direction.has_value()) {
            constraints.inheritedProperties = {
                .direction = *layoutInput.direction
            };
        }
        
        
        if (layoutInput.position == Position::Fixed || layoutInput.position == Position::Absolute) {
            lr = resolveOutOfFlow(constraints, constraints.cursor, layoutInput, atomized);
        }else {
            lr = resolveNormalFlow(constraints, constraints.cursor, layoutInput, atomized);
        }
        
        return lr;
    }
}
