#include "grid.hpp"
#include "gui/element.hpp"
#include "gui/sizing.hpp"
#include <numeric>
#include <optional>

namespace NewArch {
    void GridLayout::addChild(TreeNode* node) {
        auto gridPlacement = getGridPlacement(node);

        std::optional<int> cs, ce, rs, re = 0;

        if (gridPlacement.colStart != 0) {
            cs = gridPlacement.colStart - 1;

            if (gridPlacement.colEnd != 0) {
                ce = gridPlacement.colEnd;
            }else {
                ce = *cs + 1;
            }
        }

        if (gridPlacement.rowStart != 0) {
            rs = gridPlacement.rowStart - 1;

            if (gridPlacement.rowEnd != 0) {
                re = gridPlacement.rowEnd;
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
        float n = defs.size();
        float totalGap = (n - 1) * gap;
        float usable = available - totalGap;

        std::vector<float> sizes {};
        std::vector<Track> tracks {};

        float fixedTotal {};
        float frTotal {};

        // resolve fixed tracks
        for (size_t i = 0; i < n; ++i) {
            auto& def = defs[i];
            auto defSize = def.resolveOr(0.0);;
            if (def.isFr()) {
                frTotal += defSize;
            }else if (!def.isAuto()) {
                sizes[i] = defSize;
                fixedTotal += defSize;
                usable -= defSize;
            }
        }

        // resolve auto
        std::vector<float> autoSizes (n);
        size_t currentTrack {};

        for (auto [item, itemSize]: std::ranges::views::zip(items, itemSizes)) {
            size_t s = isCol ? *item.colStart : *item.rowStart;
            size_t e = isCol ? *item.colStart : *item.colEnd;

            if (e - s > 1)
                continue;

            if (!defs[s].isAuto())
                continue;

            // auto& cl = *node->children[item.]->layout;
            sizes[s] = std::max(sizes[s], itemSize); 
            autoSizes[s] = sizes[s];
        }

        float autoTotal = std::ranges::fold_left(autoSizes, 0.0f, std::plus<>());

        // resolve fr
        float remaining = std::max(0.0f, usable - fixedTotal - autoTotal);

        if (frTotal > 0) {
            for (size_t t = 0; t < n; ++t)
                if (defs[t].isFr()) sizes[t] = (defs[t].value / frTotal) * remaining;
        }

        float offset = 0;
        for (size_t t = 0; t < n; ++t) {
            tracks.push_back({offset, sizes[t]});
            offset += sizes[t] + gap;
        }

        return tracks;
    }

    void GridLayout::resolve(size_t numRows, size_t numCols, std::vector<Size>& templateRows, std::vector<Size>& templateCols) {
        GridLayout::resolveStructure(numRows, numCols);

        // init fixed tracks
        std::vector<Size> rowDefs(grid.numRows);
        std::vector<Size> colDefs(grid.numCols);

        for (int i = 0; i < templateRows.size(); ++i)
            rowDefs[i] = templateRows[i];

        for (int j = 0; j < templateCols.size(); ++j) 
            colDefs[j] = templateCols[j];

        // rowTracks = resolveTracks(rowDefs);
        // colTracks = resolveTracks(colDefs);
    }
}