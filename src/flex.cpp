#include "flex.hpp"
#include "render_tree.hpp"
#include "render_tree.hpp"
#include <print>

namespace layout {
    Alignment distributeSpace(float remainingSpace, size_t itemCount, DistributeMode mode) {
        Alignment a;
        switch (mode) {
            case DistributeMode::FlexStart: break;
            case DistributeMode::FlexEnd:
                a.initialOffset = remainingSpace; break;
            case DistributeMode::Center:
                a.initialOffset = remainingSpace / 2.0f; break;
            case DistributeMode::SpaceBetween:
                if (remainingSpace <= 0.0f) break;
                if (itemCount > 1) a.spaceBetween = remainingSpace / (itemCount - 1); break;
            case DistributeMode::SpaceAround: {
                if (remainingSpace <= 0.0f) break;
                float gap = remainingSpace / itemCount;
                a.initialOffset = gap / 2.0f;
                a.spaceBetween = gap;
                break;
            }
            case DistributeMode::SpaceEvenly: {
                if (remainingSpace <= 0.0f) break;
                float gap = remainingSpace / (itemCount + 1);
                a.initialOffset = gap;
                a.spaceBetween = gap;
                break;
            }
        }
        return a;
    }

    DistributeMode toDistributeMode(JustifyContent jc) {
        switch (jc) {
            case JustifyContent::FlexStart:    return DistributeMode::FlexStart;
            case JustifyContent::FlexEnd:      return DistributeMode::FlexEnd;
            case JustifyContent::Center:       return DistributeMode::Center;
            case JustifyContent::SpaceBetween: return DistributeMode::SpaceBetween;
            case JustifyContent::SpaceAround:  return DistributeMode::SpaceAround;
            case JustifyContent::SpaceEvenly:  return DistributeMode::SpaceEvenly;
        }
    }

    DistributeMode toDistributeMode(AlignContent ac) {
        switch (ac) {
            case AlignContent::Stretch:      return DistributeMode::FlexStart;
            case AlignContent::FlexStart:    return DistributeMode::FlexStart;
            case AlignContent::FlexEnd:      return DistributeMode::FlexEnd;
            case AlignContent::Center:       return DistributeMode::Center;
            case AlignContent::SpaceBetween: return DistributeMode::SpaceBetween;
            case AlignContent::SpaceAround:  return DistributeMode::SpaceAround;
            case AlignContent::SpaceEvenly:  return DistributeMode::SpaceEvenly;
        }
    }

    float layoutIntrinsicMain(
        RenderTree& tree,
        TreeNode* child,
        const FrameInfo& frameInfo,
        Constraints constraints,
        Measured measured,
        const AxisHelper& axis,
        AxisResolution mode
    ) {
        constraints.shrinkToFit = true;
        axis.setMainResolution(constraints, mode);
        axis.clearMainExplicit(measured);
        constraints.inlineFormatting = buildIsolatedInlineBoxes(
            child,
            constraints.availableWidth,
            constraints.widthResolution
        );

        auto output = tree.speculateLayout(
            child,
            frameInfo,
            constraints,
            measured
        );
        return axis.mainSize(output.layout);
    }

    void FlexResolver::prepareChildConstraints(TreeNode* child) {
        childConstraints.inlineFormatting = buildIsolatedInlineBoxes(
            child,
            childAvailableWidth,
            childConstraints.widthResolution
        );
        childConstraints.availableWidth = childAvailableWidth;
        childConstraints.inheritedProperties = parentConstraints.inheritedProperties;
    }

    void FlexResolver::phaseA() {
        if (!hasIndefiniteChild) return;

        for (uint64_t i = 0; i < node->children.size(); ++i) {
            auto childAsPtr = node->children[i].get();
            bool xIndef = isXIndefinite(childAsPtr);
            bool yIndef = isYIndefinite(childAsPtr);

            Measured childMeasured = *childAsPtr->measured;
            if (xIndef) childMeasured.explicitWidth = std::nullopt;
            if (yIndef) childMeasured.explicitHeight = std::nullopt;

            prepareChildConstraints(childAsPtr);
            bool isIndef = xIndef || yIndef;
            bool savedShrink = childConstraints.shrinkToFit;
            if (isIndef) childConstraints.shrinkToFit = true;
            
            auto childOutput = tree.speculateLayout(
                childAsPtr,
                frameInfo,
                childConstraints,
                childMeasured
            );
            auto& childLayout = childOutput.layout;

            maxIntrinsicX = std::max(maxIntrinsicX, childLayout.computedBox.x + childLayout.computedBox.width);
            maxIntrinsicY = std::max(maxIntrinsicY, childLayout.computedBox.y + childLayout.consumedHeight);

            childConstraints.shrinkToFit = savedShrink;
        }
    }

    void FlexResolver::phaseB() {
        float gapBasis = flex.axis.isRow ? parentAvailableWidth : parentAvailableHeight;
        float resolvedGap = node->getFlexGap().resolveOr(gapBasis);

        for (uint64_t i = 0; i < node->children.size(); ++i) {
            auto childAsPtr = node->children[i].get();
            auto selfAlign = childAsPtr->getAlignSelf();

            AlignItems effectiveAlign = flex.alignItems;
            if (selfAlign != AlignSelf::Auto) {
                switch (selfAlign) {
                    case AlignSelf::Stretch:   effectiveAlign = AlignItems::Stretch; break;
                    case AlignSelf::FlexStart: effectiveAlign = AlignItems::FlexStart; break;
                    case AlignSelf::FlexEnd:   effectiveAlign = AlignItems::FlexEnd; break;
                    case AlignSelf::Center:    effectiveAlign = AlignItems::Center; break;
                    default: break;
                }
            }

            prepareChildConstraints(childAsPtr);

            Measured childMeasured = *childAsPtr->measured;
            bool xIndef = isXIndefinite(childAsPtr);
            bool yIndef = isYIndefinite(childAsPtr);
            if (xIndef && !flex.axis.isRow) {
                childMeasured.explicitWidth = maxIntrinsicX - minX;
            }
            if (yIndef && flex.axis.isRow) {
                childMeasured.explicitHeight = maxIntrinsicY - minY;
            }

            auto savedCrossResolution = flex.axis.crossResolution(childConstraints);
            bool resolvingIntrinsicCross =
                savedCrossResolution == AxisResolution::MinContent ||
                savedCrossResolution == AxisResolution::MaxContent;
            const auto& crossSizeRequest = flex.axis.isRow
                ? childAsPtr->shared.height
                : childAsPtr->shared.width;
            if (!resolvingIntrinsicCross &&
                effectiveAlign == AlignItems::Stretch &&
                !flex.axis.hasUserCrossSize(childAsPtr->shared)) {
                flex.axis.setCrossResolution(childConstraints, AxisResolution::Deferred);
            }
            auto childOutput = tree.speculateLayout(
                childAsPtr,
                frameInfo,
                childConstraints,
                childMeasured
            );
            flex.axis.setCrossResolution(childConstraints, savedCrossResolution);

            auto& childLayout = childOutput.layout;
            if (childLayout.outOfFlow) continue;

            float childCrossSize = flex.axis.crossSize(childLayout);

            const auto& mainSizeRequest = flex.axis.isRow
                ? childAsPtr->shared.width
                : childAsPtr->shared.height;
            std::optional<float> preferredMainSize;
            if (mainSizeRequest.has_value()) {
                preferredMainSize = mainSizeRequest->resolve(avMain);
            }

            float flexBaseSize = preferredMainSize.has_value()
                ? *preferredMainSize
                : layoutIntrinsicMain(
                    tree,
                    childAsPtr,
                    frameInfo,
                    childConstraints,
                    childMeasured,
                    flex.axis,
                    AxisResolution::MaxContent
                );

            float resolvedGrow = childAsPtr->getFlexGrow().resolveOr(0.0, 0.0);
            float resolvedShrink = childAsPtr->getFlexShrink().resolveOr(0.0, 1.0);

            const auto& minMainRequest = flex.axis.isRow
                ? childAsPtr->shared.minWidth
                : childAsPtr->shared.minHeight;
            float minMain;
            if (minMainRequest.has_value()) {
                minMain = minMainRequest->resolveOr(avMain, 0.0f);
            } else if (childAsPtr->shared.overflow != Overflow::Visible) {
                minMain = 0.0f;
            } else {
                minMain = layoutIntrinsicMain(
                    tree,
                    childAsPtr,
                    frameInfo,
                    childConstraints,
                    childMeasured,
                    flex.axis,
                    AxisResolution::MinContent
                );
                if (preferredMainSize.has_value()) {
                    minMain = std::min(minMain, *preferredMainSize);
                }
            }

            const auto& maxMainRequest = flex.axis.isRow
                ? childAsPtr->shared.maxWidth
                : childAsPtr->shared.maxHeight;
            std::optional<float> maxMain;
            if (maxMainRequest.has_value()) {
                maxMain = maxMainRequest->resolveOr(avMain, flexBaseSize);
            }

            flex.addChild(
                flexBaseSize,
                childCrossSize,
                resolvedGrow,
                resolvedShrink,
                selfAlign,
                crossSizeRequest,
                avMain,
                resolvedGap,
                minMain,
                maxMain
            );
            inFlowIndices.push_back(i);
        }
    }

    FlexResolver::Bounds FlexResolver::phaseC() {
        float gapBasis = flex.axis.isRow ? parentAvailableWidth : parentAvailableHeight;
        float resolvedGap = node->getFlexGap().resolveOr(gapBasis);

        float totalSizeFallback = 0;
        bool resolvingMinContent =
            flex.axis.mainResolution(parentConstraints) == AxisResolution::MinContent;
        for (auto& line : flex.lines) {
            totalSizeFallback += resolvingMinContent
                ? line.totalMinimumWithGap(resolvedGap)
                : line.totalWithGap(resolvedGap);
        }
        totalSizeFallback += resolvingMinContent
            ? flex.currentLine.totalMinimumWithGap(resolvedGap)
            : flex.currentLine.totalWithGap(resolvedGap);
        auto explicitMain = flex.axis.isRow ? measured.explicitWidth : measured.explicitHeight;
        float availableMain = explicitMain.has_value()
            ? (flex.axis.isRow ? parentAvailableWidth : parentAvailableHeight)
            : totalSizeFallback;
        if (node->shared.overflow == Overflow::Scroll) {
            availableMain = std::max(availableMain, totalSizeFallback);
        }
        flex.availableMain = availableMain;

        auto resolved = flex.resolveSizes(availableMain, resolvedGap);

        float naturalCross = 0;

        for (auto& line : flex.lines) naturalCross += line.maxCrossSize;
        if (flex.lines.size() > 1) naturalCross += resolvedGap * (flex.lines.size() - 1);
        auto explicitCross = flex.axis.isRow ? measured.explicitHeight : measured.explicitWidth;
        bool fillsAvailableWidth =
            !flex.axis.isRow &&
            parentConstraints.widthResolution == AxisResolution::Final &&
            !parentConstraints.shrinkToFit;
        float availableCross = explicitCross.has_value() || fillsAvailableWidth
            ? (flex.axis.isRow ? parentAvailableHeight : parentAvailableWidth)
            : naturalCross;

        auto placements = flex.computePlacements(resolved, availableCross, resolvedGap);
        childConstraints.shrinkToFit = false;

        for (size_t pi = 0; pi < inFlowIndices.size(); ++pi) {
            size_t i = inFlowIndices[pi];
            auto childAsPtr = node->children[i].get();
            auto& p = placements[pi];
            Measured childMeasured = *childAsPtr->measured;

            prepareChildConstraints(childAsPtr);

            flex.axis.setMainPosition(childConstraints, p.mainOffset);
            flex.axis.setMainAvailableSize(childConstraints, p.mainSize);
            flex.axis.setMainResolution(childConstraints, AxisResolution::Deferred);
            flex.axis.setMainExplicit(childMeasured, p.mainSize);

            flex.axis.setCrossAvailableSize(childConstraints, availableCross);
            auto crossResolution = flex.axis.crossResolution(childConstraints);
            if (crossResolution != AxisResolution::MinContent &&
                crossResolution != AxisResolution::MaxContent) {
                flex.axis.setCrossResolution(childConstraints, AxisResolution::Final);
            }
            childConstraints.shrinkToFit = p.needsCrossShrinkToFit;

            // Cross-axis: apply placement offset and stretch override.
            // Check the original spec (shared), not measured, because measured
            // can be corrupted by Phase B from an earlier intermediate pass.
            flex.axis.setCrossPosition(childConstraints, p.crossOffset);
            if (p.crossSizeOverride.has_value()) {
                if (!flex.axis.hasUserCrossSize(childAsPtr->shared)) {
                    flex.axis.setCrossExplicit(
                        childMeasured,
                        flex.axis.clampCrossSize(*p.crossSizeOverride, childAsPtr->shared, availableCross)
                    );
                }
            }

            childConstraints.inlineFormatting = buildIsolatedInlineBoxes(
                childAsPtr,
                childConstraints.availableWidth,
                childConstraints.widthResolution
            );

            auto childOutput = mutate
                ? tree.layoutPhase(childAsPtr, frameInfo, childConstraints, childMeasured)
                : tree.speculateLayout(childAsPtr, frameInfo, childConstraints, childMeasured);
            auto& childLayout = childOutput.layout;

            maxX = std::max(maxX, childLayout.computedBox.x + childLayout.computedBox.width);
            maxY = std::max(maxY, childLayout.computedBox.y + childLayout.consumedHeight);
        }

        return {maxX, maxY};
    }

}
