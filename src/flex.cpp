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
        AxisHelper& axis,
        AxisResolution mode
    ) {
        axis.mainShrinkToFit(constraints) = true;
        axis.mainResolution(constraints) = mode;
        axis.mainExplicit(measured) = std::unexpected(SizeResolveFailure::Auto);

        constraints.inlineFormatting = buildIsolatedInlineBoxes(
            child,
            constraints.availableWidth,
            constraints.widthResolution
        );

        const auto& output = tree.speculateLayout(
            frameInfo,
            child,
            constraints,
            measured
        );
        return axis.mainSize(output.layout);
    }

    Constraints FlexResolver::prepareChildConstraints(TreeNode* child) {
        auto newChildConstraints = childConstraints;

        newChildConstraints.inlineFormatting = buildIsolatedInlineBoxes(
            child,
            childAvailableWidth,
            newChildConstraints.widthResolution
        );
        newChildConstraints.availableWidth = childAvailableWidth;
        newChildConstraints.inheritedProperties = parentConstraints.inheritedProperties;

        return newChildConstraints;
    }

    void FlexResolver::phaseA() {
        if (!hasIndefiniteChild) return;

        for (uint64_t i = 0; i < node->children.size(); ++i) {
            auto childNode = node->children[i].get();
            bool xIndef = isXIndefinite(childNode);
            bool yIndef = isYIndefinite(childNode);

            // mark sizes as indefinite if they are indefinite
            Measured childMeasured = *childNode->measured;
            if (xIndef) {
                childMeasured.explicitWidth =
                    std::unexpected(SizeResolveFailure::IndefiniteBasis);
            }
            if (yIndef) {
                childMeasured.explicitHeight =
                    std::unexpected(SizeResolveFailure::IndefiniteBasis);
            }

            auto preparedChildConstraints = prepareChildConstraints(childNode);
            const auto& childOutput = tree.speculateLayout(
                frameInfo,
                childNode,
                preparedChildConstraints,
                childMeasured
            );
            auto& childLayout = childOutput.layout;

            maxChildRight = std::max(maxChildRight, childLayout.computedBox.x + childLayout.computedBox.width);
            maxChildBottom = std::max(maxChildBottom, childLayout.computedBox.y + childLayout.consumedHeight);
        }
    }

    float FlexResolver::determineFlexBaseSize(
        TreeNode* child,
        std::expected<float, SizeResolveFailure>& mainSize,
        Constraints& constraints,
        Measured& measured
    ) {
        if (mainSize) return *mainSize;

        // fr has no flex main-size semantics yet and follows the
        // existing automatic-size fallback.
        return layoutIntrinsicMain(
            tree,
            child,
            frameInfo,
            constraints,
            measured,
            flex.axis,
            AxisResolution::MaxContent
        );
    }

    float FlexResolver::determineMinMainSize(
        TreeNode* child,
        std::expected<float, SizeResolveFailure>& mainSize,
        Constraints& constraints,
        Measured& measured
    ) {
        auto resolvedMinMain = resolveMainSize(
            flex.axis.minMainSize(child->shared)
        );
        if (resolvedMinMain) return *resolvedMinMain;

        if (resolvedMinMain.error() ==
            SizeResolveFailure::FractionRequiresContext) {
            // fr does not make sense as a flex min-size.
            return 0.0f;
        }

        if (child->shared.overflow != Overflow::Visible) return 0.0f;

        float minMainSize = layoutIntrinsicMain(
            tree,
            child,
            frameInfo,
            constraints,
            measured,
            flex.axis,
            AxisResolution::MinContent
        );
        if (mainSize) {
            minMainSize = std::min(minMainSize, *mainSize);
        }
        return minMainSize;
    }

    std::optional<float> FlexResolver::determineMaxMainSize(TreeNode* child)
    {
        const auto& request = flex.axis.maxMainSize(child->shared);
        if (!request.has_value()) return std::nullopt;

        auto resolved = resolveMainSize(*request);
        if (!resolved) return std::nullopt;
        return *resolved;
    }

    float FlexResolver::determineAvailableMain(float contentMainSize)
    {
        const auto& mainSize = flex.axis.mainExplicit(measured);

        float availableMain;
        if (mainSize) {
            availableMain = parentAvailableMain();
        } else {
            switch (mainSize.error()) {
                case SizeResolveFailure::Auto:
                    availableMain =
                        flex.axis.isRow &&
                        flex.axis.mainResolution(parentConstraints) ==
                            AxisResolution::Final &&
                        !parentConstraints.shrinkWidthToFit
                            ? parentAvailableMain()
                            : contentMainSize;
                    break;
                case SizeResolveFailure::IndefiniteBasis:
                    availableMain = contentMainSize;
                    break;
                case SizeResolveFailure::FractionRequiresContext:
                    // fr does not make sense as a flex container main size.
                    availableMain = contentMainSize;
                    break;
            }
        }

        if (node->shared.overflow == Overflow::Scroll) {
            availableMain = std::max(availableMain, contentMainSize);
        }

        return availableMain;
    }

    float FlexResolver::determineAvailableCross(
        float contentCrossSize
    ) {
        auto& crossSize = flex.axis.crossExplicit(measured);

        if (crossSize) {
            return parentAvailableCross();
        }

        switch (crossSize.error()) {
            case SizeResolveFailure::Auto:
                if (!flex.axis.isRow &&
                    flex.axis.crossResolution(parentConstraints) ==
                        AxisResolution::Final &&
                    !parentConstraints.shrinkWidthToFit) {
                    return parentAvailableCross();
                }
                return contentCrossSize;
            case SizeResolveFailure::IndefiniteBasis:
                return contentCrossSize;
            case SizeResolveFailure::FractionRequiresContext:
                // fr does not make sense as a flex container cross size.
                return contentCrossSize;
        }
    }

    void FlexResolver::phaseB() {
        float resolvedGap = node->getFlexGap()
            .resolve(Size::px(parentAvailableMain()))
            .value_or(0.0f);

        for (uint64_t i = 0; i < node->children.size(); ++i) {
            auto childAsPtr = node->children[i].get();
            auto selfAlign = childAsPtr->getAlignSelf();
            auto mainSize = resolveMainSize(childAsPtr);
            auto resolvedCrossSize = resolveCrossSize(childAsPtr);
            AlignItems effectiveAlign = flex.effectiveAlign(selfAlign);

            auto preparedChildConstraints = prepareChildConstraints(childAsPtr);

            Measured childMeasured = *childAsPtr->measured;
            if (!resolvedCrossSize &&
                resolvedCrossSize.error() == SizeResolveFailure::IndefiniteBasis) {
                flex.axis.crossExplicit(childMeasured) = flex.axis.isRow
                    ? maxChildBottom - minY
                    : maxChildRight - minX;
            }

            // we are currently resolving our intrinsic size
            auto crossResolution = flex.axis.crossResolution(preparedChildConstraints);
            bool resolvingIntrinsicCross = crossResolution == AxisResolution::MinContent || crossResolution == AxisResolution::MaxContent;

            bool crossSizeCanDeferToStretch = !resolvedCrossSize &&
                (resolvedCrossSize.error() == SizeResolveFailure::Auto ||
                resolvedCrossSize.error() == SizeResolveFailure::IndefiniteBasis);

            // if we aren't currently resolving our intrinsic cross
            // and we can stretch (auto/indefintie sizing)
            // then we want to defer sizing to phase C
            if (!resolvingIntrinsicCross &&
                effectiveAlign == AlignItems::Stretch &&
                crossSizeCanDeferToStretch
            ) {
                flex.axis.crossResolution(preparedChildConstraints) = AxisResolution::Deferred;
            }

            const auto& childOutput = tree.speculateLayout(
                frameInfo,
                childAsPtr,
                preparedChildConstraints,
                childMeasured
            );

            auto& childLayout = childOutput.layout;

            if (childLayout.outOfFlow) continue;

            float crossSize = flex.axis.crossSize(childLayout);

            float flexBaseSize = determineFlexBaseSize(
                childAsPtr,
                mainSize,
                preparedChildConstraints,
                childMeasured
            );

            float resolvedGrow = childAsPtr->getFlexGrow().resolveOr(Size::px(0.0f), 0.0f);
            float resolvedShrink = childAsPtr->getFlexShrink().resolveOr(Size::px(0.0f), 1.0f);

            float minMainSize = determineMinMainSize(
                childAsPtr,
                mainSize,
                preparedChildConstraints,
                childMeasured
            );

            auto maxMainSize = determineMaxMainSize(childAsPtr);

            flex.addChild(
                flexBaseSize,
                crossSize,
                resolvedGrow,
                resolvedShrink,
                selfAlign,
                flex.axis.crossSize(childAsPtr->shared),
                parentAvailableMain(),
                resolvedGap,
                minMainSize,
                maxMainSize
            );

            inFlowIndices.push_back(i);
        }

        if (flex.currentLine.count() > 0) {
            flex.lines.push_back(std::move(flex.currentLine));
            flex.currentLine = FlexLine{};
        }
    }

    FlexResolver::Bounds FlexResolver::phaseC() {
        float resolvedGap = node->getFlexGap()
            .resolve(Size::px(parentAvailableMain()))
            .value_or(0.0f);

        // size fallback if content based
        float totalSizeFallback = 0;
        bool resolvingMinContent = flex.axis.mainResolution(parentConstraints) == AxisResolution::MinContent;

        for (auto& line : flex.lines) {
            totalSizeFallback += resolvingMinContent
                ? line.totalMinimumWithGap(resolvedGap)
                : line.totalWithGap(resolvedGap);
        }

        float availableMain = determineAvailableMain(totalSizeFallback);
        auto resolved = flex.resolveSizes(availableMain, resolvedGap);

        float naturalCross = 0;

        for (auto& line : flex.lines) naturalCross += line.maxCrossSize;
        if (flex.lines.size() > 1) naturalCross += resolvedGap * (flex.lines.size() - 1);
        float availableCross = determineAvailableCross(naturalCross);

        auto placements = flex.computePlacements(
            resolved,
            availableMain,
            availableCross,
            resolvedGap
        );

        for (size_t pi = 0; pi < inFlowIndices.size(); ++pi) {
            size_t i = inFlowIndices[pi];
            auto childNode = node->children[i].get();
            auto& p = placements[pi];
            Measured childMeasured = *childNode->measured;

            auto preparedChildConstraints = prepareChildConstraints(childNode);

            auto childPosition = flex.axis.toPhysical(
                p.mainOffset,
                p.crossOffset
            );

            preparedChildConstraints.origin = childPosition;
            preparedChildConstraints.cursor = childPosition;
            flex.axis.mainAvailable(preparedChildConstraints) = p.mainSize;
            flex.axis.mainResolution(preparedChildConstraints) = AxisResolution::Deferred;
            flex.axis.mainExplicit(childMeasured) = p.mainSize;

            flex.axis.crossAvailable(preparedChildConstraints) = p.crossSize;
            auto crossResolution = flex.axis.crossResolution(preparedChildConstraints);
            if (crossResolution != AxisResolution::MinContent &&
                crossResolution != AxisResolution::MaxContent) {
                flex.axis.crossResolution(preparedChildConstraints) = AxisResolution::Final;
            }

            flex.axis.crossShrinkToFit(preparedChildConstraints) =
                p.needsCrossShrinkToFit;

            if (p.crossSizeOverride.has_value()) {
                flex.axis.crossExplicit(childMeasured) =
                    flex.axis.clampCrossSize(
                        *p.crossSizeOverride,
                        childNode->shared,
                        availableCross
                    );
            }

            preparedChildConstraints.inlineFormatting = buildIsolatedInlineBoxes(
                childNode,
                preparedChildConstraints.availableWidth,
                preparedChildConstraints.widthResolution
            );

            std::optional<LayoutOutput> finalChildOutput;
            const LayoutOutput* childOutput;
            if (mutate) {
                finalChildOutput = tree.layoutPhase(
                    childNode,
                    frameInfo,
                    preparedChildConstraints,
                    childMeasured
                );
                childOutput = &*finalChildOutput;
            } else {
                childOutput = &tree.speculateLayout(
                    frameInfo,
                    childNode,
                    preparedChildConstraints,
                    childMeasured
                );
            }
            const auto& childLayout = childOutput->layout;

            maxX = std::max(maxX, childLayout.computedBox.x + childLayout.computedBox.width);
            maxY = std::max(maxY, childLayout.computedBox.y + childLayout.consumedHeight);
        }

        return {maxX, maxY};
    }

}
