
#pragma once

#include "new_arch.hpp"
#include "element.hpp"

namespace NewArch {
    struct RenderTree;

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

        float mainSize(const LayoutResult& lr) const {
            return isRow ? lr.computedBox.width : lr.computedBox.height;
        }

        float crossSize(const LayoutResult& lr) const {
            return isRow ? lr.computedBox.height : lr.computedBox.width;
        }

        void setMainExplicit(Measured& m, float v) const {
            if (isRow) m.explicitWidth = v;
            else       m.explicitHeight = v;
        }
        void setCrossExplicit(Measured& m, float v) const {
            if (isRow) m.explicitHeight = v;
            else       m.explicitWidth = v;
        }
        std::optional<float> crossExplicit(const Measured& m) const {
            return isRow ? m.explicitHeight : m.explicitWidth;
        }

        void setMainPosition(Constraints& c, float v) const {
            if (isRow) {
                c.origin.x = v;
                c.cursor.x = v;
            } else {
                c.origin.y = v;
                c.cursor.y = v;
            }
        }
        void setCrossPosition(Constraints& c, float v) const {
            if (isRow) {
                c.origin.y = v;
                c.cursor.y = v;
            } else {
                c.origin.x = v;
                c.cursor.x = v;
            }
        }

        void setMainMaxSize(Constraints& c, float v) const {
            if (isRow) c.maxWidth = v;
            else       c.maxHeight = v;
        }
        void setCrossMaxSize(Constraints& c, float v) const {
            if (isRow) c.maxHeight = v;
            else       c.maxWidth = v;
        }

        float availableMain(const Measured& m, float fallback) const {
            return isRow ? m.explicitWidth.value_or(fallback) : m.explicitHeight.value_or(fallback);
        }
        float availableCross(const Measured& m, float fallback) const {
            return isRow ? m.explicitHeight.value_or(fallback) : m.explicitWidth.value_or(fallback);
        }
    };


    struct FlexLine {
        std::vector<float> childSizes;
        std::vector<float> minMainSizes;
        std::vector<float> shrinkScaled;
        std::vector<float> growthScaled;

        float totalSize{};
        float shrinkScaledTotal{};
        float growthScaledTotal{};
        float maxCrossSize{};

        void addChild(float mainSize, float crossSize, float grow, float shrink, float minMainSize = 0.0f) {
            childSizes.push_back(mainSize);
            minMainSizes.push_back(minMainSize);
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

        size_t count() const { return childSizes.size(); }

        struct ResolveResult {
            std::vector<float> sizes;
            float totalAfter{};
            float remainingSpace{};
        };

        ResolveResult resolve(float availableMain) const {
            ResolveResult result;
            float space = availableMain - totalSize;

            for (size_t i = 0; i < childSizes.size(); ++i) {
                if (space > 0 && growthScaled[i] > 0) {
                    result.sizes.push_back(childSizes[i] + (growthScaled[i] / growthScaledTotal) * space);
                } else if (space < 0 && shrinkScaled[i] > 0) {
                    float shrunk = childSizes[i] + (shrinkScaled[i] / shrinkScaledTotal) * space;
                    result.sizes.push_back(std::max(shrunk, minMainSizes[i]));
                } else {
                    result.sizes.push_back(childSizes[i]);
                }
                result.totalAfter += result.sizes.back();
            }
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
        std::vector<float> childCrossSizes;
        std::vector<std::optional<Size>> childCrossSizeRequests;
        float availableMain{};

        struct ChildPlacement {
            float mainOffset;
            float crossOffset;
            float mainSize;
            std::optional<float> crossSizeOverride; 
            bool needsCrossShrinkToFit{false};
        };

        FlexLayout() {}

        FlexLayout(FlexDirection dir, JustifyContent jc, AlignItems ai,
                    AlignContent ac, FlexWrap wrap):
            axis{dir}, justifyContent{jc}, alignItems{ai},
            alignContent{ac}, flexWrap{wrap}
        {}

        void addChild(const LayoutResult& layout, float grow, float shrink,
                        AlignSelf selfAlign, std::optional<Size> crossSizeRequest,
                        float avMain, float minMainSize = 0.0f) {
            float childMain = axis.mainSize(layout);

            float childCross = axis.crossSize(layout);


            availableMain = avMain;

            if (flexWrap != FlexWrap::NoWrap && currentLine.count() > 0) {
                float totalWithGap = currentLine.totalSize;
                if (totalWithGap + childMain > avMain) {
                    lines.push_back(std::move(currentLine));
                    currentLine = FlexLine{};
                }
            }

            currentLine.addChild(childMain, childCross, grow, shrink, minMainSize);
            childAlignSelfs.push_back(selfAlign);
            childCrossSizes.push_back(childCross);
            childCrossSizeRequests.push_back(crossSizeRequest);
        }

        struct ResolveResult {
            std::vector<std::vector<float>> lineSizes; 
            std::vector<float> lineTotalsAfter;       
            float overallTotalAfter{};
        };

        ResolveResult resolveSizes(float avMain, float gap = 0.0f) {
            if (currentLine.count() > 0) {
                lines.push_back(std::move(currentLine));
                currentLine = FlexLine{};
            }

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
            float availableCross, float gap
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

                float remainingCross = availableCross - totalNaturalCross;

                if (alignContent == AlignContent::Stretch && remainingCross > 0) {
                    float extra = remainingCross / lineCount;
                    for (size_t li = 0; li < lineCount; ++li) {
                        lineCrossSizes[li] += extra;
                    }
                    float crossAccum = 0;
                    for (size_t li = 0; li < lineCount; ++li) {
                        lineCrossOffsets[li] = crossAccum;
                        crossAccum += lineCrossSizes[li];
                    }
                } else {
                    auto crossAlign = distributeSpace(
                        remainingCross, lineCount, toDistributeMode(alignContent));
                    float crossAccum = crossAlign.initialOffset;
                    for (size_t li = 0; li < lineCount; ++li) {
                        lineCrossOffsets[li] = crossAccum;
                        crossAccum += lineCrossSizes[li] + crossAlign.spaceBetween;
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
                    AlignItems effectiveAlign = alignItems;
                    if (selfAlign != AlignSelf::Auto) {
                        switch (selfAlign) {
                            case AlignSelf::Stretch:   effectiveAlign = AlignItems::Stretch; break;
                            case AlignSelf::FlexStart: effectiveAlign = AlignItems::FlexStart; break;
                            case AlignSelf::FlexEnd:   effectiveAlign = AlignItems::FlexEnd; break;
                            case AlignSelf::Center:    effectiveAlign = AlignItems::Center; break;
                            default: break;
                        }
                    }

                    float childCross = childCrossSizeRequests[childIdx]
                        .and_then([lineCross](const Size& size) { return size.resolve(lineCross); })
                        .value_or(childCrossSizes[childIdx]);
                    switch (effectiveAlign) {
                        case AlignItems::Stretch:
                            p.crossOffset = lineCrossBase;
                            p.crossSizeOverride = lineCross;
                            break;
                        case AlignItems::FlexStart:
                            p.crossOffset = lineCrossBase;
                            p.needsCrossShrinkToFit = !axis.isRow;
                            break;
                        case AlignItems::Center:
                            p.crossOffset = lineCrossBase + (lineCross - childCross) / 2.0f;
                            p.needsCrossShrinkToFit = !axis.isRow;
                            break;
                        case AlignItems::FlexEnd:
                            p.crossOffset = lineCrossBase + lineCross - childCross;
                            p.needsCrossShrinkToFit = !axis.isRow;
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
        RenderTree&       tree;
        TreeNode*         node;
        Constraints&      parentConstraints;
        Constraints       childConstraints;
        FlexLayout        flex;
        const FrameInfo&  frameInfo;
        float             childMaxWidth;
        float             avMain;
        float             parentMaxWidth;
        float             parentMaxHeight;

        float minX;
        float minY;
        float maxX;
        float maxY;

        float maxIntrinsicX = 0;
        float maxIntrinsicY = 0;

        bool hasIndefiniteChild = false;

        struct Bounds {
            float maxX;
            float maxY;
        };

        std::vector<size_t> inFlowIndices;

        FlexResolver(RenderTree& tree, TreeNode* node, Constraints& parentConstraints,
                        Constraints childConstraints, FlexLayout flex, const FrameInfo& frameInfo,
                        float parentMaxWidth, float parentMaxHeight,
                        float minX, float minY, float maxX, float maxY)
            : tree{tree}, node{node}, parentConstraints{parentConstraints},
                childConstraints{std::move(childConstraints)}, flex{std::move(flex)},
                frameInfo{frameInfo},
                childMaxWidth{parentMaxWidth},
                avMain{this->flex.axis.isRow ? parentMaxWidth : parentMaxHeight},
                parentMaxWidth{parentMaxWidth}, parentMaxHeight{parentMaxHeight},
                minX{minX}, minY{minY}, maxX{maxX}, maxY{maxY}
        {
            bool needsCrossShrink = this->flex.axis.isRow
                || this->flex.alignItems != AlignItems::Stretch
                || !node->measured->explicitWidth.has_value();

            this->childConstraints.shrinkToFit = needsCrossShrink;

            for (auto& child : node->children) {
                if (isXIndefinite(child.get()) || isYIndefinite(child.get())) {
                    hasIndefiniteChild = true;
                    break;
                }
            }
        }

        bool isXIndefinite(TreeNode* child) const {
            return !node->measured->explicitWidth.has_value() &&
                (child->shared.width.has_value() && child->shared.width->unit == Unit::Percent);
        }

        bool isYIndefinite(TreeNode* child) const {
            return !node->measured->explicitHeight.has_value() &&
                (child->shared.height.has_value() && child->shared.height->unit == Unit::Percent);
        }

        void prepareChildConstraints(TreeNode* child);
        void phaseA();
        void phaseB();
        void phaseAB();
        void phaseC();
        Bounds phaseD();
    };
}
