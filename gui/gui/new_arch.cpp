//
//  new_arch.cpp
//  gui
//
//  Created by Taanish Reja on 11/20/25.
//

#include "new_arch.hpp"
#include "fragment_types.hpp"
#include "sizing.hpp"
#include <algorithm>
#include <optional>
#include <print>
#include <simd/vector_types.h>

namespace layout {

    simd_float2 resolvePosition(const PositionResolutionContext& ctx) {
        simd_float2 resolvedPosition = ctx.currentCursor;

        switch (ctx.layoutInput.position) {
            case layout::Position::Fixed: {
                float refWidth = ctx.constraints.frameInfo.width;
                float refHeight = ctx.constraints.frameInfo.height;

                std::optional<float> left = ctx.layoutInput.left.has_value()
                    ? ctx.layoutInput.left->resolve(refWidth) : std::nullopt;
                std::optional<float> top = ctx.layoutInput.top.has_value()
                    ? ctx.layoutInput.top->resolve(refHeight) : std::nullopt;

                resolvedPosition = {0.0f, 0.0f};

                // Only resolve left/top during layout.
                // right/bottom depend on element size and are resolved in postLayout.
                if (left.has_value()) {
                    resolvedPosition.x = *left + ctx.margins.left;
                }
                if (top.has_value()) {
                    resolvedPosition.y = *top + ctx.margins.top;
                }

                break;
            }
            case layout::Position::Absolute: {
                auto& cb = ctx.constraints.absoluteContainingBlock;
                float refWidth = cb.width;
                float refHeight = cb.height;

                std::optional<float> left = ctx.layoutInput.left.has_value()
                    ? ctx.layoutInput.left->resolve(refWidth) : std::nullopt;
                std::optional<float> top = ctx.layoutInput.top.has_value()
                    ? ctx.layoutInput.top->resolve(refHeight) : std::nullopt;

                resolvedPosition = {0.0f, 0.0f};

                // Only resolve left/top during layout.
                // right/bottom depend on element size and are resolved in postLayout.
                if (left.has_value()) {
                    resolvedPosition.x = *left + ctx.margins.left;
                }
                if (top.has_value()) {
                    resolvedPosition.y = *top + ctx.margins.top;
                }

                break;
            }   
            case layout::Position::Relative:
            case layout::Position::Static: {
                switch (ctx.layoutInput.display) {
                    case layout::Display::Flex:
                    case layout::Display::Block: {
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
                            startingX = ctx.constraints.origin.x + ctx.constraints.maxWidth - ctx.layoutInput.width.value_or(ctx.constraints.maxWidth) - ctx.margins.right;
                        }

                        resolvedPosition = {startingX, startingY};
                        break;
                    }
                    case layout::Display::Inline: {
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

    ResolvedSize resolveSize(const SizeResolutionContext& ctx)
    {
        ResolvedSize resolvedSize;

        switch (ctx.position) {
            case layout::Position::Absolute:
            case layout::Position::Fixed: {
                if (ctx.requestedHeight.has_value()) {
                    resolvedSize.height = ctx.requestedHeight->resolveOr(ctx.availableHeight, 0.0f);
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
                        resolvedSize.height = ctx.availableHeight - *resolvedTop - *resolvedBottom;
                    }
                }

                if (ctx.requestedWidth.has_value()) {
                    resolvedSize.width = ctx.requestedWidth->resolveOr(ctx.availableWidth, 0.0);
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
                        resolvedSize.width = ctx.availableWidth - *resolvedRight - * resolvedLeft;
                    }
                }

                break;
            }
            default: {
                // static fallthrough 
                if (ctx.requestedHeight.has_value()) {
                    resolvedSize.height = ctx.requestedHeight->resolveOr(ctx.availableHeight, 0.0f);
                }
                // }else {
                //     explicitSize.y = 0;
                // }

                if (ctx.requestedWidth) {
                    resolvedSize.width = ctx.requestedWidth->resolveOr(ctx.availableWidth, 0.0f);
                }
                    // }else {
                //     explicitSize.x = 0;
                // }
            }
        };

        return resolvedSize;
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

}

namespace runtime {

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

        this->frameInfo = frameInfo;
        std::memcpy(frameInfoBuffer.get()->contents(), &frameInfo, sizeof(FrameInfo));
    };

    void UIContext::updateView() {
        auto frameDimensions = this->view->drawableSize();

        auto scale = AppKit_Extensions::getContentScaleFactor(reinterpret_cast<void*>(view));

        FrameInfo frameInfo {.width=static_cast<float>(frameDimensions.width)/2.0f, .height=static_cast<float>(frameDimensions.height)/2.0f, .scale = scale};
        this->frameInfo = frameInfo;

        std::memcpy(frameInfoBuffer.get()->contents(), &frameInfo, sizeof(FrameInfo));
    }

}

namespace layout {
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
            .origin = {0, 0},
            .cursor = {0, 0},
            .maxWidth = layoutInput.width.has_value() ? resolvedWidth - paddingLeft - paddingRight : constraints.absoluteContainingBlock.width,
            .maxHeight = resolvedHeight - paddingTop - paddingBottom,
            .frameInfo = constraints.frameInfo
        };

        lr.computedBox = {
            .x = absolutePosition.x,
            .y = absolutePosition.y,
            .width = resolvedWidth,
            .height = resolvedHeight
        };

        lr.resolvedPadding = {paddingTop, paddingRight, paddingBottom, paddingLeft};

        // Defer right/bottom positioning to postLayout where final sizes are known
        auto& cb = constraints.absoluteContainingBlock;
        bool isRtl = constraints.inheritedProperties.direction == Direction::rtl;
        std::optional<float> right = layoutInput.right.has_value()
            ? layoutInput.right->resolve(cb.width) : std::nullopt;
        std::optional<float> bottom = layoutInput.bottom.has_value()
            ? layoutInput.bottom->resolve(cb.height) : std::nullopt;

        lr.deferredPosition = {
            .needsRightResolution = right.has_value() && (!layoutInput.left.has_value() || isRtl),
            .needsBottomResolution = bottom.has_value() && !layoutInput.top.has_value(),
            .containingBlockWidth = cb.width,
            .containingBlockHeight = cb.height,
            .resolvedRight = right.value_or(0.0f),
            .resolvedBottom = bottom.value_or(0.0f),
            .marginRight = margins.right,
            .marginBottom = margins.bottom,
            .direction = constraints.inheritedProperties.direction
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
            .origin = {0, 0},
            .cursor = {0, 0},
            .maxWidth = layoutInput.width.has_value() ? resolvedWidth - paddingLeft - paddingRight : constraints.frameInfo.width,
            .maxHeight = resolvedHeight - paddingTop - paddingBottom,
            .frameInfo = constraints.frameInfo
        };

        lr.computedBox = {
            .x = fixedPosition.x,
            .y = fixedPosition.y,
            .width = resolvedWidth,
            .height = resolvedHeight
        };

        lr.resolvedPadding = {paddingTop, paddingRight, paddingBottom, paddingLeft};

        // Defer right/bottom positioning to postLayout where final sizes are known
        float refWidth = constraints.frameInfo.width;
        float refHeight = constraints.frameInfo.height;
        bool isRtl = constraints.inheritedProperties.direction == Direction::rtl;
        std::optional<float> right = layoutInput.right.has_value()
            ? layoutInput.right->resolve(refWidth) : std::nullopt;
        std::optional<float> bottom = layoutInput.bottom.has_value()
            ? layoutInput.bottom->resolve(refHeight) : std::nullopt;

        lr.deferredPosition = {
            .needsRightResolution = right.has_value() && (!layoutInput.left.has_value() || isRtl),
            .needsBottomResolution = bottom.has_value() && !layoutInput.top.has_value(),
            .containingBlockWidth = refWidth,
            .containingBlockHeight = refHeight,
            .resolvedRight = right.value_or(0.0f),
            .resolvedBottom = bottom.value_or(0.0f),
            .marginRight = margins.right,
            .marginBottom = margins.bottom,
            .direction = constraints.inheritedProperties.direction
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

        // float resolvedWidth = 0;
        float resolvedHeight = 0;

        float paddingLeft = layoutInput.paddingLeft.resolveOr(constraints.maxWidth);
        float paddingTop = layoutInput.paddingTop.resolveOr(constraints.maxHeight);
        float paddingRight = layoutInput.paddingRight.resolveOr(constraints.maxWidth);
        float paddingBottom = layoutInput.paddingBottom.resolveOr(constraints.maxHeight);

        childConstraints.cursor.x = 0;
        childConstraints.cursor.y = 0;
        childConstraints.origin = childConstraints.cursor;
        childConstraints.frameInfo = constraints.frameInfo;

        for (auto& atom : atomized.atoms) {
            atomOffsets.push_back(newCursor);
            newCursor.x += atom.width;
            resolvedHeight = std::max(resolvedHeight, atom.height);
        }

        float resolvedWidth = layoutInput.width.has_value() ? 
        *layoutInput.width
            : 
        constraints.maxWidth;

        if (layoutInput.height.has_value()) {
            resolvedHeight = *layoutInput.height;
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

        lr.resolvedPadding = {paddingTop, paddingRight, paddingBottom, paddingLeft};

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

        lr.childConstraints.origin = {0.0f, 0.0f};
        float lineHeight = 0;
        float totalHeight = 0;
        float totalWidth = 0;
        float currentTotalWidth = 0;
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
            
            minX = std::min(minX, newCursor.x);

            if (fragmentIdx == 0) {

                float inlineMargin = isLtr ? margins.right : margins.left;

                if (constraints.edgeIntent.edgeDisplayMode == Display::Inline) {
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

            if (fragment.lineBoxIndex != prevLineBoxIndex &&
                prevLineBoxIndex != -1
            ) {
                newCursor.y += lineHeight;
                totalHeight += lineHeight;
                lineHeight = 0;
                newCursor.x = startingX;
                totalWidth = std::max(currentTotalWidth, totalWidth);
                currentTotalWidth = 0;
            }


            for (size_t i = 0; i < fragment.atomCount && atomIndex < atomized.atoms.size(); ++i, ++atomIndex) {
                auto& atom = atomized.atoms[atomIndex];

                atomOffsets.push_back(newCursor);
                newCursor.x += atom.width;
                lineHeight = std::max(lineHeight, atom.height);
                currentTotalWidth += atom.width;
            }

            prevLineBoxIndex = fragment.lineBoxIndex;
        }

        newCursor.x += isLtr ? margins.right : -margins.left;

        totalHeight += lineHeight;
        totalWidth = std::max(currentTotalWidth, totalWidth);

        lr.computedBox = {
            minX,
            minY,
            totalWidth,
            totalHeight
        };

        float paddingLeft = layoutInput.paddingLeft.resolveOr(constraints.maxWidth);
        float paddingTop = layoutInput.paddingTop.resolveOr(constraints.maxHeight);
        float paddingRight = layoutInput.paddingRight.resolveOr(constraints.maxWidth);
        float paddingBottom = layoutInput.paddingBottom.resolveOr(constraints.maxHeight);

        lr.childConstraints.cursor = {0, 0};
        lr.childConstraints.maxWidth = totalWidth - paddingLeft - paddingRight;
        lr.childConstraints.maxHeight = totalHeight - paddingTop - paddingBottom;
        lr.childConstraints.frameInfo = constraints.frameInfo;

        lr.atomOffsets = atomOffsets;
        lr.consumedHeight = totalHeight;

        lr.siblingCursor = newCursor;

        lr.edgeIntent = {
            .edgeDisplayMode = Display::Inline,
            .intent = isLtr ? margins.right : margins.left,
            .collapsable = false,
        };

        lr.resolvedPadding = {paddingTop, paddingRight, paddingBottom, paddingLeft};

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

        if (layoutInput.display == Display::Block || layoutInput.display == Display::Flex || layoutInput.display == Display::Grid) {
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
