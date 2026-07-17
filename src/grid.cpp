#include "grid.hpp"
#include "render_tree.hpp"
#include <algorithm>
#include <optional>
#include <print>

namespace layout {
    namespace {
        float applyMinMax(float value, const std::optional<Size>& minSize, const std::optional<Size>& maxSize, float referenceSize) {
            if (maxSize.has_value()) {
                value = std::min(value, maxSize->resolveOr(referenceSize, value));
            }
            if (minSize.has_value()) {
                value = std::max(value, minSize->resolveOr(referenceSize, value));
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
            auto& def = defs[i];
            bool indefinite = !axisDefinite && (def.unit == Unit::Percent || def.isFr());

            if (indefinite || def.isAuto()) {
                sizes[i] = trackMinSizes[i];
                autoTotal += sizes[i];
            } else if (def.isFr()) {
                frTotal += def.value;
            } else {
                sizes[i] = def.resolveOr(available, 0);
                fixedTotal += sizes[i];
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

    GridResolver::GridResolver(RenderTree& tree, TreeNode* node, Constraints& parentConstraints,
                               Constraints childConstraints, const FrameInfo& frameInfo,
                               Measured measured, bool mutate,
                               float parentAvailableWidth, float parentAvailableHeight,
                               float minX, float minY, float maxX, float maxY)
        : tree{tree}, node{node}, parentConstraints{parentConstraints},
          childConstraints{std::move(childConstraints)},
          alignItems{node->getAlignItems()},
          frameInfo{frameInfo}, measured{measured}, mutate{mutate},
          childAvailableWidth{measured.explicitWidth.value_or(parentConstraints.availableWidth)},
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

    bool GridResolver::isXIndefinite(TreeNode* child) const {
        return !measured.explicitWidth.has_value() &&
            child->shared.width.has_value() && (child->shared.width->unit == Unit::Percent || child->shared.width->unit == Unit::Fr);
    }

    bool GridResolver::isYIndefinite(TreeNode* child) const {
        return !measured.explicitHeight.has_value() &&
            child->shared.height.has_value() && (child->shared.height->unit == Unit::Percent || child->shared.height->unit == Unit::Fr);
    }

    void GridResolver::prepareChildConstraints(TreeNode* child) {
        childConstraints.inlineFormatting = buildIsolatedInlineBoxes(
            child,
            childAvailableWidth,
            childConstraints.widthResolution
        );
        childConstraints.availableWidth = childAvailableWidth;
        childConstraints.inheritedProperties = parentConstraints.inheritedProperties;
    }

    void GridResolver::phaseA() {
        if (!hasIndefiniteChild) return;

        std::println("here in a!");

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

    void GridResolver::phaseB() {
        auto& templateCols = node->getGridTemplateColumns();
        auto& templateRows = node->getGridTemplateRows();
        float availableWidth = measured.explicitWidth.value_or(parentConstraints.availableWidth);
        float availableHeight = measured.explicitHeight.value_or(parentConstraints.availableHeight);
        float colGap = node->getGridColumnGap().resolveOr(availableWidth, 0);
        float rowGap = node->getGridRowGap().resolveOr(availableHeight, 0);

        std::vector<float> itemWidths;
        std::vector<float> itemHeights;

        for (size_t i = 0; i < node->children.size(); ++i) {
            auto childAsPtr = node->children[i].get();
            auto childPos = childAsPtr->getPosition();
            if (childPos == Position::Absolute || childPos == Position::Fixed) continue;

            Measured childMeasured = *childAsPtr->measured;
            if (isXIndefinite(childAsPtr)) {
                childMeasured.explicitWidth = maxIntrinsicX - minX;
            }
            if (isYIndefinite(childAsPtr)) {
                childMeasured.explicitHeight = maxIntrinsicY - minY;
            }

            prepareChildConstraints(childAsPtr);
            if (!hasIndefiniteChild) {
                childConstraints.shrinkToFit = true;
            }

            auto childOutput = tree.speculateLayout(
                childAsPtr,
                frameInfo,
                childConstraints,
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
                availableWidth
            );
            float itemHeight = applyMinMax(
                childLayout.consumedHeight,
                childAsPtr->shared.minHeight,
                childAsPtr->shared.maxHeight,
                availableHeight
            );

            itemWidths.push_back(itemWidth);
            itemHeights.push_back(itemHeight);
        }

        float trackAvailableWidth = parentConstraints.shrinkToFit && !measured.explicitWidth.has_value()
            ? 0.0f
            : parentAvailableWidth;
        float trackAvailableHeight = parentConstraints.shrinkToFit && !measured.explicitHeight.has_value()
            ? 0.0f
            : parentAvailableHeight;

        gridLayout.resolve(templateRows.size(), templateCols.size(),
            templateRows, templateCols,
            trackAvailableWidth, trackAvailableHeight,
            colGap, rowGap,
            itemWidths, itemHeights,
            measured.explicitWidth.has_value(), measured.explicitHeight.has_value());
    }

    GridResolver::Bounds GridResolver::phaseC() {
        float availableWidth = measured.explicitWidth.value_or(parentConstraints.availableWidth);
        float availableHeight = measured.explicitHeight.value_or(parentConstraints.availableHeight);

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

            childConstraints.inlineFormatting = buildIsolatedInlineBoxes(
                childAsPtr,
                itemW,
                childConstraints.widthResolution
            );
            childConstraints.availableWidth = itemW;
            childConstraints.availableHeight = itemH;
            childConstraints.origin = {cellX, cellY};
            childConstraints.cursor = {cellX, cellY};
            childConstraints.inheritedProperties = parentConstraints.inheritedProperties;
            childConstraints.shrinkToFit = false;

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
                if (!childAsPtr->shared.width.has_value())
                    childMeasured.explicitWidth = itemW;
                if (!childAsPtr->shared.height.has_value())
                    childMeasured.explicitHeight = itemH;
            } else {
                childConstraints.shrinkToFit = true;
            }

            auto childOutput = tree.speculateLayout(
                childAsPtr,
                frameInfo,
                childConstraints,
                childMeasured
            );

            float dx = 0.0f;
            float dy = 0.0f;
            if (effectiveAlign == AlignItems::Center) {
                dx = (cellW - childOutput.layout.computedBox.width) / 2.0f;
                dy = (cellH - childOutput.layout.computedBox.height) / 2.0f;
            } else if (effectiveAlign == AlignItems::FlexEnd) {
                dx = cellW - childOutput.layout.computedBox.width;
                dy = cellH - childOutput.layout.computedBox.height;
            }

            childConstraints.origin.x += dx;
            childConstraints.origin.y += dy;
            childConstraints.cursor.x += dx;
            childConstraints.cursor.y += dy;

            if (mutate) {
                childOutput = tree.layoutPhase(
                    childAsPtr,
                    frameInfo,
                    childConstraints,
                    childMeasured
                );
            } else if (dx != 0.0f || dy != 0.0f) {
                childOutput = tree.speculateLayout(
                    childAsPtr,
                    frameInfo,
                    childConstraints,
                    childMeasured
                );
            }

            auto& childLayout = childOutput.layout;

            maxX = std::max(maxX, childLayout.computedBox.x + childLayout.computedBox.width);
            maxY = std::max(maxY, childLayout.computedBox.y + childLayout.consumedHeight);
        }

        return {maxX, maxY};
    }
}
