#include "grid.hpp"
#include "render_tree.hpp"
#include <algorithm>
#include <optional>

namespace layout {
    namespace {
        float applyMinMax(float value, const Size& minSize, const std::optional<Size>& maxSize, float referenceSize) {
            auto basis = Size::px(referenceSize);

            if (maxSize.has_value()) {
                auto resolvedMax = maxSize->resolve(basis);
                if (resolvedMax) {
                    value = std::min(value, *resolvedMax);
                }
            }

            auto resolvedMin = minSize.resolve(basis);
            if (resolvedMin) {
                value = std::max(value, *resolvedMin);
            }

            return value;
        }
    }

    void GridLayout::addChild(TreeNode* node) {
        auto gridPlacement = node->getGridPlacement();

        std::optional<int> cs, ce, rs, re;

        if (gridPlacement.colStart != 0) {
            cs = gridPlacement.colStart - 1;

            if (gridPlacement.colEnd != 0) {
                ce = gridPlacement.colEnd - 1;
            }else {
                ce = *cs + 1;
            }
        }

        if (gridPlacement.rowStart != 0) {
            rs = gridPlacement.rowStart - 1;

            if (gridPlacement.rowEnd != 0) {
                re = gridPlacement.rowEnd - 1;
            }else {
                re = *rs + 1;
            }
        }

        ItemPlacement item {
            .colStart = cs,
            .colEnd = ce,
            .rowStart = rs,
            .rowEnd = re
        };

        items.push_back(item);
    }

    Grid::Grid(size_t rows, size_t cols, GridDirection major): 
        occupied{rows, std::vector<uint8_t>(cols, 0)},
        numRows{rows},
        numCols{cols}, 
        majorAxis{major}
    {}

    void Grid::mark(int row, int col) {
        occupied[row][col] = 1;
    }

    bool Grid::regionFree(int row, int col, int spanRows, int spanCols) const {
        for (int r = row; r < row + spanRows; ++r)
            for (int c = col; c < col + spanCols; ++c)
                if (occupied[r][c]) return false;

        
        return true;
    }

    void Grid::growMajor(int needed) {
        if (majorAxis == GridDirection::Row) {
            while (numRows < needed) {
                occupied.push_back(std::vector<uint8_t>(numCols, 0));
                numRows++;
            }
        } else {
            while (numCols < needed) {
                for (auto& row : occupied)
                    row.push_back(0);
                numCols++;
            }
        }
    }

    void Grid::advanceCursor(int spanMinor) {
        cursorMinor += spanMinor;

        if (cursorMinor >= minorSize()) {
            cursorMinor = 0;
            cursorMajor++;
        }

    }

    int Grid::majorSize() const {
        return (majorAxis == GridDirection::Row) ? numRows : numCols;
    }

    int Grid::minorSize() const {
        return (majorAxis == GridDirection::Row) ? numCols : numRows;
    }

    std::pair<int, int> Grid::findSpace(int spanRows, int spanCols) {
        bool rowMajor = (majorAxis == GridDirection::Row);
        int spanMajor = rowMajor ? spanRows : spanCols;
        int spanMinor = rowMajor ? spanCols : spanRows;

        while (true) {
            if (cursorMinor + spanMinor > minorSize()) {
                cursorMinor = 0;
                cursorMajor++;
            }
            
            if (cursorMajor + spanMajor > majorSize()) {
                growMajor(cursorMajor + spanMajor);
            }

            int row = rowMajor ? cursorMajor : cursorMinor;
            int col = rowMajor ? cursorMinor : cursorMajor;

            if (regionFree(row, col, spanRows, spanCols)) {
                advanceCursor(spanMinor);
                return {row, col};
            }

            cursorMinor++;
        }
    }


    void GridLayout::resolveStructure(size_t numRows, size_t numCols) {
        grid = Grid{numRows, numCols};

        // place explicitly placed items : who wins if items conflict in explicit positions?
        for (auto& item : items) {
            if (!item.colNeedsResolution() && !item.rowNeedsResolution()) {
                for (int r = *item.rowStart; r < *item.rowEnd; ++r)
                    for (int c = *item.colStart; c < *item.colEnd; ++c)
                        grid.mark(r, c);
            }
        }

        // place items with unresolved placements
        for (auto& item : items) {
            if (!item.colNeedsResolution() && !item.rowNeedsResolution()) continue;

            int spanCols = item.colNeedsResolution() ? 1 : (*item.colEnd - *item.colStart);
            int spanRows = item.rowNeedsResolution() ? 1 : (*item.rowEnd - *item.rowStart);

            auto [row, col] = grid.findSpace(spanRows, spanCols);

            if (item.colNeedsResolution()) {
                item.colStart = col;
                item.colEnd = col + spanCols;
            }
            if (item.rowNeedsResolution()) {
                item.rowStart = row;
                item.rowEnd = row + spanRows;
            }

            for (int r = *item.rowStart; r < *item.rowEnd; ++r)
                for (int c = *item.colStart; c < *item.colEnd; ++c)
                    grid.mark(r, c);
        }
    }


    std::vector<Track> GridLayout::resolveTracks(std::vector<Size>& defs, std::vector<float> itemSizes, float available, float gap, bool isCol, bool axisDefinite) {
        size_t n = defs.size();
        float totalGap = (n > 1) ? gap * (float)(n - 1) : 0;
        float usable = available - totalGap;

        std::vector<float> sizes(n, 0);
        std::vector<float> trackMinSizes(n, 0);
        std::vector<Track> tracks {};

        float fixedTotal {};
        float frTotal {};
        float autoTotal {};

        for (auto [item, itemSize]: std::ranges::views::zip(items, itemSizes)) {
            size_t s = isCol ? *item.colStart : *item.rowStart;
            size_t e = isCol ? *item.colEnd : *item.rowEnd;

            if (e - s > 1)
                continue;

            trackMinSizes[s] = std::max(trackMinSizes[s], itemSize);
        }

        // resolve fixed tracks
        for (size_t i = 0; i < n; ++i) {
            auto basis = axisDefinite
                ? Size::px(available)
                : Size::autoSize();
            auto resolved = defs[i].resolve(basis);

            if (resolved) {
                sizes[i] = *resolved;
                fixedTotal += *resolved;
                continue;
            }

            switch (resolved.error()) {
                case SizeResolveFailure::Auto:
                case SizeResolveFailure::IndefiniteBasis:
                    sizes[i] = trackMinSizes[i];
                    autoTotal += sizes[i];
                    break;
                case SizeResolveFailure::FractionRequiresContext:
                    if (axisDefinite) {
                        frTotal += defs[i].value;
                    } else {
                        sizes[i] = trackMinSizes[i];
                        autoTotal += sizes[i];
                    }
                    break;
            }
        }

        // resolve fr
        float remaining = std::max(0.0f, usable - fixedTotal - autoTotal);

        if (frTotal > 0) {
            for (size_t i = 0; i < n; ++i)
                if (defs[i].isFr()) sizes[i] = (defs[i].value / frTotal) * remaining;
        }

        float offset = 0;
        for (size_t t = 0; t < n; ++t) {
            tracks.push_back({offset, sizes[t]});
            offset += sizes[t] + gap;
        }

        return tracks;
    }

    void GridLayout::resolve(
        size_t numRows, size_t numCols,
        const std::vector<Size>& templateRows, const std::vector<Size>& templateCols,
        float availableWidth, float availableHeight,
        float colGap, float rowGap,
        std::vector<float> itemWidths, std::vector<float> itemHeights,
        bool widthDefinite, bool heightDefinite) {
        GridLayout::resolveStructure(numRows, numCols);

        // init fixed tracks
        std::vector<Size> rowDefs(grid.numRows, Size::autoSize());
        std::vector<Size> colDefs(grid.numCols, Size::autoSize());

        for (int i = 0; i < templateRows.size(); ++i)
            rowDefs[i] = templateRows[i];

        for (int j = 0; j < templateCols.size(); ++j)
            colDefs[j] = templateCols[j];

        colTracks = resolveTracks(colDefs, itemWidths, availableWidth, colGap, true, widthDefinite);
        rowTracks = resolveTracks(rowDefs, itemHeights, availableHeight, rowGap, false, heightDefinite);
    }

    GridResolver::GridResolver(RenderTree& tree, TreeNode* node,
                               const Constraints& parentConstraints,
                               const Constraints& childConstraints,
                               const FrameInfo& frameInfo,
                               Measured measured, bool mutate,
                               float parentAvailableWidth, float parentAvailableHeight,
                               float minX, float minY, float maxX, float maxY)
        : tree{tree}, node{node}, parentConstraints{parentConstraints},
          childConstraints{childConstraints},
          alignItems{node->getAlignItems()},
          frameInfo{frameInfo}, measured{measured}, mutate{mutate},
          childAvailableWidth{parentAvailableWidth},
          parentAvailableWidth{parentAvailableWidth}, parentAvailableHeight{parentAvailableHeight},
          minX{minX}, minY{minY}, maxX{maxX}, maxY{maxY}
    {
        for (auto& child : node->children) {
            if (isXIndefinite(child.get()) || isYIndefinite(child.get())) {
                hasIndefiniteChild = true;
                break;
            }
        }
    }

    bool GridResolver::isXIndefinite(TreeNode* child) {
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
            resolvedWidth.error() == SizeResolveFailure::IndefiniteBasis;
    }

    bool GridResolver::isYIndefinite(TreeNode* child) {
        bool basisIsIndefinite =
            parentConstraints.shrinkHeightToFit ||
            parentConstraints.heightResolution == AxisResolution::MinContent ||
            parentConstraints.heightResolution == AxisResolution::MaxContent ||
            (!measured.explicitHeight &&
             (measured.explicitHeight.error() == SizeResolveFailure::Auto ||
              measured.explicitHeight.error() ==
                SizeResolveFailure::IndefiniteBasis));

        auto resolvedHeight = child->shared.height.resolve(
            basisIsIndefinite
                ? Size::autoSize()
                : Size::px(parentAvailableHeight)
        );

        return !resolvedHeight &&
            resolvedHeight.error() == SizeResolveFailure::IndefiniteBasis;
    }

    Constraints GridResolver::prepareChildConstraints(TreeNode* child) {
        auto preparedChildConstraints = childConstraints;

        preparedChildConstraints.inlineFormatting = buildIsolatedInlineBoxes(
            child,
            childAvailableWidth,
            preparedChildConstraints.widthResolution
        );
        preparedChildConstraints.availableWidth = childAvailableWidth;
        preparedChildConstraints.inheritedProperties =
            parentConstraints.inheritedProperties;

        return preparedChildConstraints;
    }

    void GridResolver::phaseA() {
        if (!hasIndefiniteChild) return;

        for (uint64_t i = 0; i < node->children.size(); ++i) {
            auto childAsPtr = node->children[i].get();
            bool xIndef = isXIndefinite(childAsPtr);
            bool yIndef = isYIndefinite(childAsPtr);

            Measured childMeasured = *childAsPtr->measured;
            if (xIndef) {
                childMeasured.explicitWidth =
                    std::unexpected(style::SizeResolveFailure::IndefiniteBasis);
            }
            if (yIndef) {
                childMeasured.explicitHeight =
                    std::unexpected(style::SizeResolveFailure::IndefiniteBasis);
            }

            auto preparedChildConstraints =
                prepareChildConstraints(childAsPtr);

            const auto& childOutput = tree.speculateLayout(
                frameInfo,
                childAsPtr,
                preparedChildConstraints,
                childMeasured
            );
            auto& childLayout = childOutput.layout;

            maxChildRight = std::max(
                maxChildRight,
                childLayout.computedBox.x + childLayout.computedBox.width
            );
            maxChildBottom = std::max(
                maxChildBottom,
                childLayout.computedBox.y + childLayout.consumedHeight
            );
        }
    }

    void GridResolver::phaseB() {
        auto& templateCols = node->getGridTemplateColumns();
        auto& templateRows = node->getGridTemplateRows();

        bool widthBasisIsIndefinite =
            parentConstraints.shrinkWidthToFit ||
            parentConstraints.widthResolution == AxisResolution::MinContent ||
            parentConstraints.widthResolution == AxisResolution::MaxContent ||
            (!measured.explicitWidth &&
             measured.explicitWidth.error() ==
                SizeResolveFailure::IndefiniteBasis);

        bool heightBasisIsIndefinite =
            parentConstraints.shrinkHeightToFit ||
            parentConstraints.heightResolution == AxisResolution::MinContent ||
            parentConstraints.heightResolution == AxisResolution::MaxContent ||
            (!measured.explicitHeight &&
             (measured.explicitHeight.error() == SizeResolveFailure::Auto ||
              measured.explicitHeight.error() ==
                SizeResolveFailure::IndefiniteBasis));

        bool widthDefinite = !widthBasisIsIndefinite;
        bool heightDefinite = !heightBasisIsIndefinite;

        auto widthBasis = widthDefinite
            ? Size::px(parentAvailableWidth)
            : Size::autoSize();
        auto heightBasis = heightDefinite
            ? Size::px(parentAvailableHeight)
            : Size::autoSize();

        float colGap = node->getGridColumnGap()
            .resolve(widthBasis)
            .value_or(0.0f);
        float rowGap = node->getGridRowGap()
            .resolve(heightBasis)
            .value_or(0.0f);

        std::vector<float> itemWidths;
        std::vector<float> itemHeights;

        for (size_t i = 0; i < node->children.size(); ++i) {
            auto childAsPtr = node->children[i].get();
            auto childPos = childAsPtr->getPosition();
            if (childPos == Position::Absolute || childPos == Position::Fixed) continue;

            Measured childMeasured = *childAsPtr->measured;
            if (isXIndefinite(childAsPtr)) {
                childMeasured.explicitWidth = maxChildRight - minX;
            }
            if (isYIndefinite(childAsPtr)) {
                childMeasured.explicitHeight = maxChildBottom - minY;
            }

            auto preparedChildConstraints =
                prepareChildConstraints(childAsPtr);
            if (!hasIndefiniteChild) {
                preparedChildConstraints.shrinkWidthToFit = true;
                preparedChildConstraints.shrinkHeightToFit = true;
            }

            const auto& childOutput = tree.speculateLayout(
                frameInfo,
                childAsPtr,
                preparedChildConstraints,
                childMeasured
            );
            auto& childLayout = childOutput.layout;
            if (childLayout.outOfFlow) 
                continue;

            gridLayout.addChild(childAsPtr);
            inFlowIndices.push_back(i);

            float itemWidth = applyMinMax(
                childLayout.computedBox.width,
                childAsPtr->shared.minWidth,
                childAsPtr->shared.maxWidth,
                parentAvailableWidth
            );
            float itemHeight = applyMinMax(
                childLayout.consumedHeight,
                childAsPtr->shared.minHeight,
                childAsPtr->shared.maxHeight,
                parentAvailableHeight
            );

            itemWidths.push_back(itemWidth);
            itemHeights.push_back(itemHeight);
        }

        gridLayout.resolve(templateRows.size(), templateCols.size(),
            templateRows, templateCols,
            parentAvailableWidth, parentAvailableHeight,
            colGap, rowGap,
            itemWidths, itemHeights,
            widthDefinite, heightDefinite);
    }

    GridResolver::Bounds GridResolver::phaseC() {
        for (size_t pi = 0; pi < inFlowIndices.size(); ++pi) {
            size_t i = inFlowIndices[pi];
            auto childAsPtr = node->children[i].get();
            auto& item = gridLayout.items[pi];
            Measured childMeasured = *childAsPtr->measured;

            auto& colTracks = gridLayout.colTracks;
            auto& rowTracks = gridLayout.rowTracks;

            float cellX = colTracks[*item.colStart].offset;
            float cellY = rowTracks[*item.rowStart].offset;

            float cellW = colTracks[*item.colEnd - 1].offset + colTracks[*item.colEnd - 1].size - cellX;
            float cellH = rowTracks[*item.rowEnd - 1].offset + rowTracks[*item.rowEnd - 1].size - cellY;

            float itemW = applyMinMax(cellW, childAsPtr->shared.minWidth, childAsPtr->shared.maxWidth, cellW);
            float itemH = applyMinMax(cellH, childAsPtr->shared.minHeight, childAsPtr->shared.maxHeight, cellH);

            auto preparedChildConstraints =
                prepareChildConstraints(childAsPtr);

            preparedChildConstraints.inlineFormatting = buildIsolatedInlineBoxes(
                childAsPtr,
                itemW,
                preparedChildConstraints.widthResolution
            );
            preparedChildConstraints.availableWidth = itemW;
            preparedChildConstraints.availableHeight = itemH;
            preparedChildConstraints.origin = {cellX, cellY};
            preparedChildConstraints.cursor = {cellX, cellY};
            preparedChildConstraints.shrinkWidthToFit = false;
            preparedChildConstraints.shrinkHeightToFit = false;

            // resolve alignment
            AlignItems effectiveAlign = alignItems;
            auto selfAlign = childAsPtr->getAlignSelf();
            if (selfAlign != AlignSelf::Auto) {
                switch (selfAlign) {
                    case AlignSelf::Stretch:   effectiveAlign = AlignItems::Stretch; break;
                    case AlignSelf::FlexStart: effectiveAlign = AlignItems::FlexStart; break;
                    case AlignSelf::FlexEnd:   effectiveAlign = AlignItems::FlexEnd; break;
                    case AlignSelf::Center:    effectiveAlign = AlignItems::Center; break;
                    default: break;
                }
            }

            // stretch: override measured size if child has no explicit size
            if (effectiveAlign == AlignItems::Stretch) {
                if (childAsPtr->shared.width.isAuto())
                    childMeasured.explicitWidth = itemW;
                if (childAsPtr->shared.height.isAuto())
                    childMeasured.explicitHeight = itemH;
            } else {
                preparedChildConstraints.shrinkWidthToFit = true;
                preparedChildConstraints.shrinkHeightToFit = true;
            }

            const LayoutOutput* childOutput = &tree.speculateLayout(
                frameInfo,
                childAsPtr,
                preparedChildConstraints,
                childMeasured
            );

            float dy = 0.0f;
            if (effectiveAlign == AlignItems::Center) {
                dy = (cellH - childOutput->layout.computedBox.height) / 2.0f;
            } else if (effectiveAlign == AlignItems::FlexEnd) {
                dy = cellH - childOutput->layout.computedBox.height;
            }

            preparedChildConstraints.origin.y += dy;
            preparedChildConstraints.cursor.y += dy;

            std::optional<LayoutOutput> finalChildOutput;
            if (mutate) {
                finalChildOutput = tree.layoutPhase(
                    childAsPtr,
                    frameInfo,
                    preparedChildConstraints,
                    childMeasured
                );
                childOutput = &*finalChildOutput;
            } else if (dy != 0.0f) {
                childOutput = &tree.speculateLayout(
                    frameInfo,
                    childAsPtr,
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
