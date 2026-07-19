
#pragma once

#include "new_arch.hpp"
#include "element.hpp"

namespace tree {
    struct RenderTree;
}

namespace layout {
    using style::AlignContent;
    using style::AlignItems;
    using style::AlignSelf;
    using style::FlexDirection;
    using style::FlexWrap;
    using style::JustifyContent;
    using style::Overflow;
    using style::Size;
    using style::SizeResolveFailure;
    using style::Unit;
    using tree::RenderTree;
    using tree::TreeNode;

    struct AxisHelper {
        bool isRow;
        bool isReversed;

        AxisHelper() {};

        AxisHelper(FlexDirection dir):
            isRow{dir == FlexDirection::Row || dir == FlexDirection::RowReverse},
            isReversed{dir == FlexDirection::RowReverse || dir == FlexDirection::ColReverse}
        {}

        void applyDirection(Direction dir) {
            if (dir == Direction::rtl && isRow) isReversed = !isReversed;
        }

        float mainSize(const LayoutResult& lr) {
            return isRow ? lr.computedBox.width : lr.computedBox.height;
        }

        float crossSize(const LayoutResult& lr) {
            return isRow ? lr.computedBox.height : lr.computedBox.width;
        }
        const Size& mainSize(const SharedDescriptor& shared) {
            return isRow ? shared.width : shared.height;
        }
        const Size& crossSize(const SharedDescriptor& shared) {
            return isRow ? shared.height : shared.width;
        }
        const Size& minMainSize(const SharedDescriptor& shared) {
            return isRow ? shared.minWidth : shared.minHeight;
        }
        const std::optional<Size>& maxMainSize(const SharedDescriptor& shared) {
            return isRow ? shared.maxWidth : shared.maxHeight;
        }
        std::expected<float, SizeResolveFailure>& mainExplicit(Measured& m) {
            return isRow ? m.explicitWidth : m.explicitHeight;
        }
        std::expected<float, SizeResolveFailure>& crossExplicit(Measured& m) {
            return isRow ? m.explicitHeight : m.explicitWidth;
        }

        simd_float2 toPhysical(float main, float cross) {
            return isRow
                ? simd_float2{main, cross}
                : simd_float2{cross, main};
        }

        float& mainAvailable(Constraints& c) {
            return isRow ? c.availableWidth : c.availableHeight;
        }
        float& crossAvailable(Constraints& c) {
            return isRow ? c.availableHeight : c.availableWidth;
        }
        AxisResolution& mainResolution(Constraints& c) {
            return isRow ? c.widthResolution : c.heightResolution;
        }
        AxisResolution& crossResolution(Constraints& c) {
            return isRow ? c.heightResolution : c.widthResolution;
        }
        bool& mainShrinkToFit(Constraints& c) {
            return isRow ? c.shrinkWidthToFit : c.shrinkHeightToFit;
        }
        bool& crossShrinkToFit(Constraints& c) {
            return isRow ? c.shrinkHeightToFit : c.shrinkWidthToFit;
        }
        float clampCrossSize(float v, const SharedDescriptor& s, float referenceSize) {
            auto& maxSize = isRow ? s.maxHeight : s.maxWidth;
            const auto& minSize = isRow ? s.minHeight : s.minWidth;
            auto basis = Size::px(referenceSize);
            if (maxSize.has_value()) {
                auto resolvedMax = maxSize->resolve(basis);
                if (resolvedMax) {
                    v = std::min(v, *resolvedMax);
                } else {
                    switch (resolvedMax.error()) {
                        case SizeResolveFailure::Auto:
                        case SizeResolveFailure::IndefiniteBasis:
                            break;
                        case SizeResolveFailure::FractionRequiresContext:
                            // fr does not make sense as a flex max cross-size.
                            break;
                    }
                }
            }
            auto resolvedMin = minSize.resolve(basis);
            if (resolvedMin) {
                v = std::max(v, *resolvedMin);
            } else {
                switch (resolvedMin.error()) {
                    case SizeResolveFailure::Auto:
                    case SizeResolveFailure::IndefiniteBasis:
                        break;
                    case SizeResolveFailure::FractionRequiresContext:
                        // fr does not make sense as a flex min cross-size.
                        break;
                }
            }
            return v;
        }

    };


    struct FlexLine {
        std::vector<float> childSizes;
        std::vector<float> hypotheticalMainSizes;
        std::vector<float> minMainSizes;
        std::vector<std::optional<float>> maxMainSizes;
        std::vector<float> shrinkScaled;
        std::vector<float> growthScaled;

        float totalSize{};
        float shrinkScaledTotal{};
        float growthScaledTotal{};
        float maxCrossSize{};

        void addChild(float mainSize, float hypotheticalMainSize, float crossSize, float grow, float shrink,
                      float minMainSize = 0.0f, std::optional<float> maxMainSize = std::nullopt) {
            childSizes.push_back(mainSize);
            hypotheticalMainSizes.push_back(hypotheticalMainSize);
            minMainSizes.push_back(minMainSize);
            maxMainSizes.push_back(maxMainSize);
            totalSize += mainSize;

            if (shrink > 0.0f) {
                shrinkScaled.push_back(mainSize * shrink);
                shrinkScaledTotal += mainSize * shrink;
            } else {
                shrinkScaled.push_back(0.0f);
            }

            if (grow > 0.0f) {
                growthScaled.push_back(grow);
                growthScaledTotal += grow;
            } else {
                growthScaled.push_back(0.0f);
            }

            maxCrossSize = std::max(crossSize, maxCrossSize);
        }

        size_t count() { return childSizes.size(); }

        float totalWithGap(float gap) {
            return totalSize + (count() > 1 ? gap * (count() - 1) : 0.0f);
        }

        float totalHypotheticalWithGap(float gap) {
            float total = 0.0f;
            for (float size : hypotheticalMainSizes) total += size;
            return total + (count() > 1 ? gap * (count() - 1) : 0.0f);
        }

        float totalMinimumWithGap(float gap) {
            float total = 0.0f;
            for (float size : minMainSizes) total += size;
            return total + (count() > 1 ? gap * (count() - 1) : 0.0f);
        }

        struct ResolveResult {
            std::vector<float> sizes;
            float totalAfter{};
            float remainingSpace{};
        };

        ResolveResult resolve(float availableMain) {
            ResolveResult result;
            result.sizes = childSizes;

            std::vector<bool> frozen(childSizes.size(), false);

            while (true) {
                float frozenTotal = 0.0f;
                float unfrozenBaseTotal = 0.0f;
                float unfrozenGrowthTotal = 0.0f;
                float unfrozenShrinkTotal = 0.0f;

                for (size_t i = 0; i < childSizes.size(); ++i) {
                    if (frozen[i]) {
                        frozenTotal += result.sizes[i];
                    } else {
                        unfrozenBaseTotal += childSizes[i];
                        unfrozenGrowthTotal += growthScaled[i];
                        unfrozenShrinkTotal += shrinkScaled[i];
                    }
                }

                float space = availableMain - frozenTotal - unfrozenBaseTotal;

                for (size_t i = 0; i < childSizes.size(); ++i) {
                    if (frozen[i]) continue;

                    if (space > 0.0f && growthScaled[i] > 0.0f && unfrozenGrowthTotal > 0.0f) {
                        result.sizes[i] = childSizes[i] + (growthScaled[i] / unfrozenGrowthTotal) * space;
                    } else if (space < 0.0f && shrinkScaled[i] > 0.0f && unfrozenShrinkTotal > 0.0f) {
                        result.sizes[i] = childSizes[i] + (shrinkScaled[i] / unfrozenShrinkTotal) * space;
                    } else {
                        result.sizes[i] = childSizes[i];
                    }
                }

                bool anyViolation = false;

                for (size_t i = 0; i < childSizes.size(); ++i) {
                    if (frozen[i]) continue;

                    float clamped = result.sizes[i];
                    if (maxMainSizes[i].has_value()) {
                        clamped = std::min(clamped, *maxMainSizes[i]);
                    }
                    clamped = std::max(clamped, minMainSizes[i]);

                    if (clamped != result.sizes[i]) {
                        result.sizes[i] = clamped;
                        frozen[i] = true;
                        anyViolation = true;
                    }
                }

                if (!anyViolation) break;
            }

            for (auto size : result.sizes) result.totalAfter += size;
            result.remainingSpace = availableMain - result.totalAfter;
            return result;
        }
    };

    struct Alignment {
        float initialOffset{};
        float spaceBetween{};
    };

    enum class DistributeMode {
        FlexStart, FlexEnd, Center, SpaceBetween, SpaceAround, SpaceEvenly
    };

    Alignment distributeSpace(float remainingSpace, size_t itemCount, DistributeMode mode);

    DistributeMode toDistributeMode(JustifyContent jc);

    DistributeMode toDistributeMode(AlignContent ac);


    struct FlexLayout {
        AxisHelper axis;
        JustifyContent justifyContent;
        AlignItems alignItems;
        AlignContent alignContent;
        FlexWrap flexWrap;

        std::vector<FlexLine> lines;
        FlexLine currentLine;
        std::vector<AlignSelf> childAlignSelfs;
        std::vector<float> crossSizes;
        std::vector<Size> childCrossSizeRequests;

        struct ChildPlacement {
            float mainOffset;
            float crossOffset;
            float mainSize;
            float crossSize;
            std::optional<float> crossSizeOverride; 
            bool needsCrossShrinkToFit{false};
        };

        FlexLayout() {}

        FlexLayout(FlexDirection dir, JustifyContent jc, AlignItems ai,
                    AlignContent ac, FlexWrap wrap):
            axis{dir}, justifyContent{jc}, alignItems{ai},
            alignContent{ac}, flexWrap{wrap}
        {}

        AlignItems effectiveAlign(AlignSelf selfAlign) {
            switch (selfAlign) {
                case AlignSelf::Auto:      return alignItems;
                case AlignSelf::Stretch:   return AlignItems::Stretch;
                case AlignSelf::FlexStart: return AlignItems::FlexStart;
                case AlignSelf::FlexEnd:   return AlignItems::FlexEnd;
                case AlignSelf::Center:    return AlignItems::Center;
            }
        }

        void addChild(float flexBaseSize, float crossSize, float grow, float shrink,
                      AlignSelf selfAlign, Size crossSizeRequest,
                      float avMain, float gap, float minMainSize = 0.0f,
                      std::optional<float> maxMainSize = std::nullopt) {
            float hypotheticalMainSize = flexBaseSize;
            if (maxMainSize.has_value()) {
                hypotheticalMainSize = std::min(hypotheticalMainSize, *maxMainSize);
            }
            hypotheticalMainSize = std::max(hypotheticalMainSize, minMainSize);

            if (flexWrap != FlexWrap::NoWrap && currentLine.count() > 0) {
                if (currentLine.totalHypotheticalWithGap(gap) + gap + hypotheticalMainSize > avMain) {
                    lines.push_back(std::move(currentLine));
                    currentLine = FlexLine{};
                }
            }

            currentLine.addChild(
                flexBaseSize,
                hypotheticalMainSize,
                crossSize,
                grow,
                shrink,
                minMainSize,
                maxMainSize
            );
            childAlignSelfs.push_back(selfAlign);
            crossSizes.push_back(crossSize);
            childCrossSizeRequests.push_back(crossSizeRequest);
        }

        struct ResolveResult {
            std::vector<std::vector<float>> lineSizes; 
            std::vector<float> lineTotalsAfter;       
            float overallTotalAfter{};
        };

        ResolveResult resolveSizes(float avMain, float gap = 0.0f) {
            ResolveResult result;
            for (auto& line : lines) {
                float lineGap = line.count() > 1 ? gap * (line.count() - 1) : 0.0f;
                auto lr = line.resolve(avMain - lineGap);
                result.overallTotalAfter += lr.totalAfter;
                result.lineTotalsAfter.push_back(lr.totalAfter);
                result.lineSizes.push_back(std::move(lr.sizes));
            }
            return result;
        }

        std::vector<ChildPlacement> computePlacements(
            const ResolveResult& resolved,
            float availableMain,
            float availableCross,
            float gap
        ) {
            size_t lineCount = lines.size();

            std::vector<float> lineCrossSizes(lineCount);
            std::vector<float> lineCrossOffsets(lineCount);

            if (lineCount == 1) {
                lineCrossSizes[0] = availableCross;
                lineCrossOffsets[0] = 0.0f;
            } else {
                float totalNaturalCross = 0;
                for (size_t li = 0; li < lineCount; ++li) {
                    lineCrossSizes[li] = lines[li].maxCrossSize;
                    totalNaturalCross += lines[li].maxCrossSize;
                }

                float crossGap = gap * (lineCount - 1);
                float remainingCross = availableCross - totalNaturalCross - crossGap;

                if (alignContent == AlignContent::Stretch && remainingCross > 0) {
                    float extra = remainingCross / lineCount;
                    for (size_t li = 0; li < lineCount; ++li) {
                        lineCrossSizes[li] += extra;
                    }
                    float crossAccum = 0;
                    for (size_t li = 0; li < lineCount; ++li) {
                        lineCrossOffsets[li] = crossAccum;
                        crossAccum += lineCrossSizes[li] + gap;
                    }
                } else {
                    auto crossAlign = distributeSpace(
                        remainingCross, lineCount, toDistributeMode(alignContent));
                    float crossAccum = crossAlign.initialOffset;
                    for (size_t li = 0; li < lineCount; ++li) {
                        lineCrossOffsets[li] = crossAccum;
                        crossAccum += lineCrossSizes[li] + crossAlign.spaceBetween + gap;
                    }
                }

                // Wrap-reverse: mirror line cross offsets
                if (flexWrap == FlexWrap::WrapReverse) {
                    for (size_t li = 0; li < lineCount; ++li) {
                        lineCrossOffsets[li] = availableCross - lineCrossOffsets[li] - lineCrossSizes[li];
                    }
                }
            }

            // Build per-child placements
            std::vector<ChildPlacement> placements;
            size_t childIdx = 0;

            for (size_t li = 0; li < lineCount; ++li) {
                auto& line = lines[li];
                float lineGap = line.count() > 1 ? gap * (line.count() - 1) : 0.0f;
                float lineRemainingMain = availableMain - resolved.lineTotalsAfter[li] - lineGap;
                auto mainAlign = distributeSpace(lineRemainingMain, line.count(), toDistributeMode(justifyContent));

                float accumulated = mainAlign.initialOffset;
                float lineCross = lineCrossSizes[li];
                float lineCrossBase = lineCrossOffsets[li];

                for (size_t ci = 0; ci < line.count(); ++ci) {
                    ChildPlacement p;
                    p.mainOffset = accumulated;
                    p.mainSize = resolved.lineSizes[li][ci];

                    AlignSelf selfAlign = childAlignSelfs[childIdx];
                    AlignItems effectiveAlign = this->effectiveAlign(selfAlign);

                    float childCross = crossSizes[childIdx];
                    const auto& crossSizeRequest = childCrossSizeRequests[childIdx];
                    auto resolvedCrossSize = crossSizeRequest.resolve(
                        Size::px(lineCross)
                    );

                    if (resolvedCrossSize) {
                        childCross = *resolvedCrossSize;
                    }

                    p.crossSize = childCross;

                    switch (effectiveAlign) {
                        case AlignItems::Stretch:
                            p.crossOffset = lineCrossBase;
                            if (crossSizeRequest.isAuto()) {
                                p.crossSize = lineCross;
                                p.crossSizeOverride = lineCross;
                            }
                            break;
                        case AlignItems::FlexStart:
                            p.crossOffset = lineCrossBase;
                            p.needsCrossShrinkToFit = axis.isRow;
                            break;
                        case AlignItems::Center:
                            p.crossOffset = lineCrossBase + (lineCross - childCross) / 2.0f;
                            p.needsCrossShrinkToFit = axis.isRow;
                            break;
                        case AlignItems::FlexEnd:
                            p.crossOffset = lineCrossBase + lineCross - childCross;
                            p.needsCrossShrinkToFit = axis.isRow;
                            break;
                    }

                    if (axis.isReversed) {
                        p.mainOffset = availableMain - accumulated - p.mainSize;
                    }

                    accumulated += resolved.lineSizes[li][ci] + mainAlign.spaceBetween + gap;
                    placements.push_back(p);
                    childIdx++;
                }
            }

            return placements;
        }
    };

    struct FlexResolver {
        RenderTree& tree;
        TreeNode* node;
        Constraints parentConstraints;
        Constraints childConstraints;
        FlexLayout flex;
        const FrameInfo& frameInfo;
        Measured measured;
        bool mutate;
        float childAvailableWidth;
        float parentAvailableWidth;
        float parentAvailableHeight;

        float minX;
        float minY;
        float maxX;
        float maxY;

        float maxChildRight = 0;
        float maxChildBottom = 0;

        bool hasIndefiniteChild = false;
        struct Bounds {
            float maxX;
            float maxY;
        };

        std::vector<size_t> inFlowIndices;

        FlexResolver(RenderTree& tree, TreeNode* node, const Constraints& parentConstraints,
                        const Constraints& childConstraints, FlexLayout flex, const FrameInfo& frameInfo,
                        Measured measured, bool mutate,
                        float parentAvailableWidth, float parentAvailableHeight,
                        float minX, float minY, float maxX, float maxY)
            : tree{tree}, node{node}, parentConstraints{parentConstraints},
                childConstraints{childConstraints}, flex{std::move(flex)},
                frameInfo{frameInfo}, measured{measured}, mutate{mutate},
                childAvailableWidth{parentAvailableWidth},
                parentAvailableWidth{parentAvailableWidth}, parentAvailableHeight{parentAvailableHeight},
                minX{minX}, minY{minY}, maxX{maxX}, maxY{maxY}
        {
            bool needsCrossShrink = this->flex.axis.isRow
                || this->flex.alignItems != AlignItems::Stretch
                || !measured.explicitWidth.has_value();

            this->flex.axis.crossShrinkToFit(this->childConstraints) =
                needsCrossShrink;

            for (auto& child : node->children) {
                if (isXIndefinite(child.get()) || isYIndefinite(child.get())) {
                    hasIndefiniteChild = true;
                    break;
                }
            }
        }

        float parentAvailableMain() {
            return flex.axis.isRow
                ? parentAvailableWidth
                : parentAvailableHeight;
        }

        float parentAvailableCross() {
            return flex.axis.isRow
                ? parentAvailableHeight
                : parentAvailableWidth;
        }

        // x and y axis indefinite checks are asymmetric
        // because of how HTML/CSS auto-expands width (think block, flex, etc...)
        // but does not do the same for height
        bool isXIndefinite(TreeNode* child) {
            /* axes are indefinite for three reasons:
                - The axis is being shrink-to-fit measured.
                - The axis is being measured as min/max-content.
                - Measured explicitly says IndefiniteBasis.
        
                this 2 trillion line if statement effectively covers this case
            */
            bool basisIsIndefinite =
                parentConstraints.shrinkWidthToFit ||
                parentConstraints.widthResolution == AxisResolution::MinContent ||
                parentConstraints.widthResolution == AxisResolution::MaxContent ||
                (!measured.explicitWidth &&
                 measured.explicitWidth.error() ==
                    SizeResolveFailure::IndefiniteBasis);

            auto resolvedWidth = child->shared.width.resolve(
                basisIsIndefinite
                    ? Size::autoSize()
                    : Size::px(parentAvailableWidth)
            );

            return !resolvedWidth &&
                resolvedWidth.error() ==
                    SizeResolveFailure::IndefiniteBasis;
        }

        bool isYIndefinite(TreeNode* child) {
            bool basisIsIndefinite =
                parentConstraints.shrinkHeightToFit ||
                parentConstraints.heightResolution == AxisResolution::MinContent ||
                parentConstraints.heightResolution == AxisResolution::MaxContent ||
                (!measured.explicitHeight &&
                 (measured.explicitHeight.error() ==
                    SizeResolveFailure::Auto ||
                  measured.explicitHeight.error() ==
                    SizeResolveFailure::IndefiniteBasis));

            auto resolvedHeight = child->shared.height.resolve(
                basisIsIndefinite
                    ? Size::autoSize()
                    : Size::px(parentAvailableHeight)
            );

            return !resolvedHeight &&
                resolvedHeight.error() ==
                    SizeResolveFailure::IndefiniteBasis;
        }

        std::expected<float, SizeResolveFailure> resolveMainSize(
            const Size& request
        ) {
            auto& mainSize = flex.axis.mainExplicit(measured);
            bool basisIsIndefinite = flex.axis.isRow
                ? parentConstraints.shrinkWidthToFit ||
                  parentConstraints.widthResolution == AxisResolution::MinContent ||
                  parentConstraints.widthResolution == AxisResolution::MaxContent
                : parentConstraints.shrinkHeightToFit ||
                  parentConstraints.heightResolution == AxisResolution::MinContent ||
                  parentConstraints.heightResolution == AxisResolution::MaxContent;
            if (!mainSize) {
                basisIsIndefinite = basisIsIndefinite ||
                    mainSize.error() ==
                        SizeResolveFailure::IndefiniteBasis ||
                    (!flex.axis.isRow &&
                     mainSize.error() == SizeResolveFailure::Auto);
            }
            auto basis = basisIsIndefinite
                ? Size::autoSize()
                : Size::px(parentAvailableMain());
            return request.resolve(basis);
        }

        std::expected<float, SizeResolveFailure> resolveMainSize(
            TreeNode* child
        ) {
            return resolveMainSize(flex.axis.mainSize(child->shared));
        }

        std::expected<float, SizeResolveFailure> resolveCrossSize(
            TreeNode* child
        ) {
            const auto& request = flex.axis.crossSize(child->shared);
            float availableCross = flex.axis.isRow
                ? parentAvailableHeight
                : parentAvailableWidth;
            auto& crossSize = flex.axis.crossExplicit(measured);
            bool basisIsIndefinite = flex.axis.isRow
                ? parentConstraints.shrinkHeightToFit ||
                  parentConstraints.heightResolution == AxisResolution::MinContent ||
                  parentConstraints.heightResolution == AxisResolution::MaxContent
                : parentConstraints.shrinkWidthToFit ||
                  parentConstraints.widthResolution == AxisResolution::MinContent ||
                  parentConstraints.widthResolution == AxisResolution::MaxContent;
            if (!crossSize) {
                basisIsIndefinite = basisIsIndefinite ||
                    crossSize.error() ==
                        SizeResolveFailure::IndefiniteBasis ||
                    (flex.axis.isRow &&
                     crossSize.error() == SizeResolveFailure::Auto);
            }
            auto basis = basisIsIndefinite
                ? Size::autoSize()
                : Size::px(availableCross);
            return request.resolve(basis);
        }

        float determineFlexBaseSize(
            TreeNode* child,
            std::expected<float, SizeResolveFailure>& mainSize,
            Constraints& constraints,
            Measured& measured
        );

        float determineMinMainSize(
            TreeNode* child,
            std::expected<float, SizeResolveFailure>& mainSize,
            Constraints& constraints,
            Measured& measured
        );

        std::optional<float> determineMaxMainSize(TreeNode* child);
        float determineAvailableMain(float contentMainSize);
        float determineAvailableCross(float contentCrossSize);

        Constraints prepareChildConstraints(TreeNode* child);
        void phaseA();
        void phaseB();
        Bounds phaseC();
    };
}
