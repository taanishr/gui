#include "grid.hpp"
#include "render_tree.hpp"
#include <optional>
#include <print>

namespace NewArch {
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


    std::vector<Track> GridLayout::resolveTracks(std::vector<Size>& defs, std::vector<float> itemSizes, float available, float gap, bool isCol) {
        size_t n = defs.size();
        float totalGap = (n > 1) ? gap * (float)(n - 1) : 0;
        float usable = available - totalGap;

        std::vector<float> sizes(n, 0);
        std::vector<float> trackMinSizes(n, 0);
        std::vector<Track> tracks {};

        float fixedTotal {};
        float frTotal {};

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
            if (def.isFr()) {
                frTotal += def.value;
            }else if (!def.isAuto()) {
                sizes[i] = std::max(def.resolveOr(available, 0), trackMinSizes[i]);
                fixedTotal += sizes[i];
            }
        }

        // resolve auto
        std::vector<float> autoSizes (n, 0);

        for (size_t i = 0; i < n; ++i) {
            if (!defs[i].isAuto())
                continue;

            sizes[i] = trackMinSizes[i];
            autoSizes[i] = sizes[i];
        }

        float autoTotal = std::ranges::fold_left(autoSizes, 0.0f, std::plus<>());

        // resolve fr
        float remaining = std::max(0.0f, usable - fixedTotal - autoTotal);

        if (frTotal > 0) {
            for (size_t i = 0; i < n; ++i)
                if (defs[i].isFr()) sizes[i] = std::max((defs[i].value / frTotal) * remaining, trackMinSizes[i]);
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
        std::vector<float> itemWidths, std::vector<float> itemHeights) {
        GridLayout::resolveStructure(numRows, numCols);

        // init fixed tracks
        std::vector<Size> rowDefs(grid.numRows, Size::autoSize());
        std::vector<Size> colDefs(grid.numCols, Size::autoSize());

        for (int i = 0; i < templateRows.size(); ++i)
            rowDefs[i] = templateRows[i];

        for (int j = 0; j < templateCols.size(); ++j)
            colDefs[j] = templateCols[j];

        colTracks = resolveTracks(colDefs, itemWidths, availableWidth, colGap, true);
        rowTracks = resolveTracks(rowDefs, itemHeights, availableHeight, rowGap, false);
    }

    GridResolver::GridResolver(RenderTree& tree, TreeNode* node, Constraints& parentConstraints,
                               Constraints childConstraints, const FrameInfo& frameInfo,
                               float parentMaxWidth, float parentMaxHeight,
                               float minX, float minY, float maxX, float maxY)
        : tree{tree}, node{node}, parentConstraints{parentConstraints},
          childConstraints{std::move(childConstraints)},
          alignItems{node->getAlignItems()},
          frameInfo{frameInfo},
          childMaxWidth{node->measured->explicitWidth.value_or(parentConstraints.maxWidth)},
          parentMaxWidth{parentMaxWidth}, parentMaxHeight{parentMaxHeight},
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
        return !node->measured->explicitWidth.has_value() &&
            child->shared.width.has_value() && (child->shared.width->unit == Unit::Percent || child->shared.width->unit == Unit::Fr);
    }

    bool GridResolver::isYIndefinite(TreeNode* child) const {
        return !node->measured->explicitHeight.has_value() &&
            child->shared.height.has_value() && (child->shared.height->unit == Unit::Percent || child->shared.height->unit == Unit::Fr);
    }

    void GridResolver::prepareChildConstraints(TreeNode* child) {
        auto&& [frags, boxes] = buildIsolatedInlineBoxes(child, childMaxWidth);
        childConstraints.lineFragments = frags;
        childConstraints.lineBoxes = boxes;
        childConstraints.maxWidth = childMaxWidth;
        childConstraints.inheritedProperties = parentConstraints.inheritedProperties;
    }

    void GridResolver::phaseA() {
        if (!hasIndefiniteChild) return;

        std::println("here in a!");

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

    void GridResolver::phaseB() {
        if (!hasIndefiniteChild) return;

        std::println("here in b!");

        for (uint64_t i = 0; i < node->children.size(); ++i) {
            auto childAsPtr = node->children[i].get();
            bool xIndef = isXIndefinite(childAsPtr);
            bool yIndef = isYIndefinite(childAsPtr);
            if (!xIndef && !yIndef) continue;

            if (xIndef) childAsPtr->measured->explicitWidth = maxIntrinsicX - minX;
            if (yIndef) childAsPtr->measured->explicitHeight = maxIntrinsicY - minY;

            prepareChildConstraints(childAsPtr);
            tree.layoutPhase(childAsPtr, frameInfo, childConstraints);
        }
    }

    void GridResolver::phaseC() {
        auto& measured = *node->measured;
        auto& templateCols = node->getGridTemplateColumns();
        auto& templateRows = node->getGridTemplateRows();
        float availableWidth = measured.explicitWidth.value_or(parentConstraints.maxWidth);
        float availableHeight = measured.explicitHeight.value_or(parentConstraints.maxHeight);
        float colGap = node->getGridColumnGap().resolveOr(availableWidth, 0);
        float rowGap = node->getGridRowGap().resolveOr(availableHeight, 0);

        std::vector<float> itemWidths;
        std::vector<float> itemHeights;

        for (size_t i = 0; i < node->children.size(); ++i) {
            auto childAsPtr = node->children[i].get();
            auto childPos = childAsPtr->getPosition();
            if (childPos == Position::Absolute || childPos == Position::Fixed) continue;

            if (!hasIndefiniteChild) {
                prepareChildConstraints(childAsPtr);
                childConstraints.shrinkToFit = true;
                tree.layoutPhase(childAsPtr, frameInfo, childConstraints);
            }

            auto& childLayout = *childAsPtr->layout;
            if (childLayout.outOfFlow) 
                continue;

            gridLayout.addChild(childAsPtr);
            inFlowIndices.push_back(i);

            itemWidths.push_back(childLayout.computedBox.width);
            itemHeights.push_back(childLayout.consumedHeight);
        }

        float trackAvailableWidth = parentConstraints.shrinkToFit && !measured.explicitWidth.has_value()
            ? 0.0f
            : parentMaxWidth;
        float trackAvailableHeight = parentConstraints.shrinkToFit && !measured.explicitHeight.has_value()
            ? 0.0f
            : parentMaxHeight;

        gridLayout.resolve(templateRows.size(), templateCols.size(),
            templateRows, templateCols,
            trackAvailableWidth, trackAvailableHeight,
            colGap, rowGap,
            itemWidths, itemHeights);

    }

    GridResolver::Bounds GridResolver::phaseD() {
        auto& measured = *node->measured;
        float availableWidth = measured.explicitWidth.value_or(parentConstraints.maxWidth);
        float availableHeight = measured.explicitHeight.value_or(parentConstraints.maxHeight);

        for (size_t pi = 0; pi < inFlowIndices.size(); ++pi) {
            size_t i = inFlowIndices[pi];
            auto childAsPtr = node->children[i].get();
            auto& item = gridLayout.items[pi];

            auto& colTracks = gridLayout.colTracks;
            auto& rowTracks = gridLayout.rowTracks;

            float cellX = colTracks[*item.colStart].offset;
            float cellY = rowTracks[*item.rowStart].offset;

            float cellW = colTracks[*item.colEnd - 1].offset + colTracks[*item.colEnd - 1].size - cellX;
            float cellH = rowTracks[*item.rowEnd - 1].offset + rowTracks[*item.rowEnd - 1].size - cellY;


            auto&& [frags, boxes] = buildIsolatedInlineBoxes(childAsPtr, cellW);
            childConstraints.lineFragments = frags;
            childConstraints.lineBoxes = boxes;
            childConstraints.maxWidth = cellW;
            childConstraints.maxHeight = cellH;
            childConstraints.origin.x = cellX;
            childConstraints.cursor.y = cellY;
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
                    childAsPtr->measured->explicitWidth = cellW;
                if (!childAsPtr->shared.height.has_value())
                    childAsPtr->measured->explicitHeight = cellH;
            } else {
                childConstraints.shrinkToFit = true;
            }

            tree.layoutPhase(childAsPtr, frameInfo, childConstraints);
            auto& childLayout = *childAsPtr->layout;

            // non-stretch alignment: offset within cell
            if (effectiveAlign == AlignItems::Center) {
                float dx = (cellW - childLayout.computedBox.width) / 2.0f;
                float dy = (cellH - childLayout.computedBox.height) / 2.0f;
                childLayout.computedBox.x += dx;
                childLayout.computedBox.y += dy;
            } else if (effectiveAlign == AlignItems::FlexEnd) {
                float dx = cellW - childLayout.computedBox.width;
                float dy = cellH - childLayout.computedBox.height;
                childLayout.computedBox.x += dx;
                childLayout.computedBox.y += dy;
            }

            maxX = std::max(maxX, childLayout.computedBox.x + childLayout.computedBox.width);
            maxY = std::max(maxY, childLayout.computedBox.y + childLayout.consumedHeight);
        }

        return {maxX, maxY};
    }
}
