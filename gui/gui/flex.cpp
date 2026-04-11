#include "flex.hpp"
#include "render_tree.hpp"

namespace NewArch {
    Alignment distributeSpace(float remainingSpace, size_t itemCount, DistributeMode mode) {
        Alignment a;
        switch (mode) {
            case DistributeMode::FlexStart: break;
            case DistributeMode::FlexEnd:
                a.initialOffset = remainingSpace; break;
            case DistributeMode::Center:
                a.initialOffset = remainingSpace / 2.0f; break;
            case DistributeMode::SpaceBetween:
                if (itemCount > 1) a.spaceBetween = remainingSpace / (itemCount - 1); break;
            case DistributeMode::SpaceAround: {
                float gap = remainingSpace / itemCount;
                a.initialOffset = gap / 2.0f;
                a.spaceBetween = gap;
                break;
            }
            case DistributeMode::SpaceEvenly: {
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

    void FlexResolver::prepareChildConstraints(TreeNode* child) {
        auto&& [frags, boxes] = buildIsolatedInlineBoxes(child, childMaxWidth);
        childConstraints.lineFragments = frags;
        childConstraints.lineBoxes = boxes;
        childConstraints.maxWidth = childMaxWidth;
        childConstraints.inheritedProperties = parentConstraints.inheritedProperties;
    }

    void FlexResolver::phaseA() {
        for (uint64_t i = 0; i < node->children.size(); ++i) {
            auto childAsPtr = node->children[i].get();
            bool xIndef = isXIndefinite(childAsPtr);
            bool yIndef = isYIndefinite(childAsPtr);

            std::optional<float> savedWidth, savedHeight;
            if (xIndef) {
                savedWidth = childAsPtr->measured->explicitWidth;
                childAsPtr->measured->explicitWidth = std::nullopt;
            }
            if (yIndef) {
                savedHeight = childAsPtr->measured->explicitHeight;
                childAsPtr->measured->explicitHeight = std::nullopt;
            }

            prepareChildConstraints(childAsPtr);
            bool isIndef = xIndef || yIndef;
            bool savedShrink = childConstraints.shrinkToFit;
            if (isIndef) childConstraints.shrinkToFit = true;

            tree.layoutPhase(childAsPtr, frameInfo, childConstraints);
            auto& childLayout = *childAsPtr->layout;

            maxIntrinsicX = std::max(maxIntrinsicX, childLayout.computedBox.x + childLayout.computedBox.width);
            maxIntrinsicY = std::max(maxIntrinsicY, childLayout.computedBox.y + childLayout.consumedHeight);

            if (xIndef) childAsPtr->measured->explicitWidth = savedWidth;
            if (yIndef) childAsPtr->measured->explicitHeight = savedHeight;
            childConstraints.shrinkToFit = savedShrink;
        }
    }

    void FlexResolver::phaseB() {
        for (uint64_t i = 0; i < node->children.size(); ++i) {
            auto childAsPtr = node->children[i].get();
            bool xIndef = isXIndefinite(childAsPtr);
            bool yIndef = isYIndefinite(childAsPtr);

            if (!xIndef && !yIndef) continue;

            bool setCrossWidth = xIndef && !flex.axis.isRow;
            bool setCrossHeight = yIndef && flex.axis.isRow;

            if (!setCrossWidth && !setCrossHeight) continue;

            if (setCrossWidth) childAsPtr->measured->explicitWidth = maxIntrinsicX - minX;
            if (setCrossHeight) childAsPtr->measured->explicitHeight = maxIntrinsicY - minY;

            // relayout step really only covers edge cases with text wrapping
            prepareChildConstraints(childAsPtr);

            tree.layoutPhase(childAsPtr, frameInfo, childConstraints);
        }
    }

    void FlexResolver::phaseAB() {
        if (hasIndefiniteChild) {
            phaseA();
            phaseB();
        }
    }

    void FlexResolver::phaseC() {
        for (uint64_t i = 0; i < node->children.size(); ++i) {
            auto childAsPtr = node->children[i].get();

            if (!hasIndefiniteChild) {
                prepareChildConstraints(childAsPtr);

                tree.layoutPhase(childAsPtr, frameInfo, childConstraints);
            }

            auto& childLayout = *childAsPtr->layout;
            if (childLayout.outOfFlow) continue;

            float resolvedGrow = getFlexGrow(childAsPtr).resolveOr(0.0, 0.0);
            float resolvedShrink = getFlexShrink(childAsPtr).resolveOr(0.0, 1.0);
            auto selfAlign = getAlignSelf(childAsPtr);

            bool hasExplicitMain = flex.axis.isRow
                ? childAsPtr->shared.width.has_value()
                : childAsPtr->shared.height.has_value();

            float minMain = hasExplicitMain ? 0.0f : flex.axis.mainSize(childLayout);

            flex.addChild(childLayout, resolvedGrow, resolvedShrink, selfAlign, avMain, minMain);
            inFlowIndices.push_back(i);
        }
    }

    FlexResolver::Bounds FlexResolver::phaseD() {
        auto& measured = *node->measured;

        float totalSizeFallback = 0;
        for (auto& line : flex.lines) totalSizeFallback += line.totalSize;
        totalSizeFallback += flex.currentLine.totalSize;
        auto explicitMain = flex.axis.isRow ? measured.explicitWidth : measured.explicitHeight;
        float availableMain = explicitMain.has_value()
            ? (flex.axis.isRow ? parentMaxWidth : parentMaxHeight)
            : totalSizeFallback;
        flex.availableMain = availableMain;

        auto resolved = flex.resolveSizes(availableMain);

        float gapBasis = flex.axis.isRow
            ? measured.explicitWidth.value_or(parentConstraints.maxWidth)
            : measured.explicitHeight.value_or(resolved.overallTotalAfter);
        float resolvedGap = getFlexGap(node).resolveOr(gapBasis);

        float naturalCross = 0;

        for (auto& line : flex.lines) naturalCross += line.maxCrossSize;
        auto explicitCross = flex.axis.isRow ? measured.explicitHeight : measured.explicitWidth;
        float availableCross = explicitCross.has_value()
            ? (flex.axis.isRow ? parentMaxHeight : parentMaxWidth)
            : naturalCross;

        auto placements = flex.computePlacements(resolved, availableCross, resolvedGap);
        childConstraints.shrinkToFit = false;

        for (size_t pi = 0; pi < inFlowIndices.size(); ++pi) {
            size_t i = inFlowIndices[pi];
            auto childAsPtr = node->children[i].get();
            auto& p = placements[pi];

            prepareChildConstraints(childAsPtr);

            flex.axis.setMainPosition(childConstraints, p.mainOffset);
            flex.axis.setMainMaxSize(childConstraints, p.mainSize);
            flex.axis.setMainExplicit(*childAsPtr->measured, p.mainSize);

            // Cross axis: col needs maxWidth constraint; row doesn't need maxHeight
            if (!flex.axis.isRow) {
                flex.axis.setCrossMaxSize(childConstraints, availableCross);
            }
            childConstraints.shrinkToFit = p.needsCrossShrinkToFit;

            // Cross-axis: apply placement offset and stretch override.
            // Check the original spec (shared), not measured, because measured
            // can be corrupted by Phase B from an earlier intermediate pass.
            flex.axis.setCrossPosition(childConstraints, p.crossOffset);
            if (p.crossSizeOverride.has_value()) {
                bool hasUserCrossSize = flex.axis.isRow
                    ? childAsPtr->shared.height.has_value()
                    : childAsPtr->shared.width.has_value();
                if (!hasUserCrossSize) {
                    flex.axis.setCrossExplicit(*childAsPtr->measured, *p.crossSizeOverride);
                }
            }

            tree.layoutPhase(childAsPtr, frameInfo, childConstraints);
            auto& childLayout = *childAsPtr->layout;

            maxX = std::max(maxX, childLayout.computedBox.x + childLayout.computedBox.width);
            maxY = std::max(maxY, childLayout.computedBox.y + childLayout.consumedHeight);
        }

        return {maxX, maxY};
    }

}
