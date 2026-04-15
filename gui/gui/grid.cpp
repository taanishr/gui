#include "grid.hpp"
#include "gui/element.hpp"
#include <optional>

void NewArch::GridLayout::addChild(NewArch::TreeNode* node) {
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

NewArch::Grid::Grid(size_t rows, size_t cols, GridDirection major): 
    occupied{rows, std::vector<uint8_t>(cols, 0)},
    numRows{rows},
    numCols{cols}, 
    majorAxis{major}
{}

void NewArch::Grid::mark(int row, int col) {
    occupied[row][col] = 1;
}

bool NewArch::Grid::regionFree(int row, int col, int spanRows, int spanCols) const {
    for (int r = row; r < row + spanRows; ++r)
        for (int c = col; c < col + spanCols; ++c)
            if (occupied[r][c]) return false;

    
    return true;
}

void NewArch::Grid::growMajor(int needed) {
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

void NewArch::Grid::advanceCursor(int spanMinor) {
    cursorMinor += spanMinor;

    if (cursorMinor >= minorSize()) {
        cursorMinor = 0;
        cursorMajor++;
    }

}

int NewArch::Grid::majorSize() const {
    return (majorAxis == GridDirection::Row) ? numRows : numCols;
}

int NewArch::Grid::minorSize() const {
    return (majorAxis == GridDirection::Row) ? numCols : numRows;
}

std::pair<int, int> NewArch::Grid::findEmptyRegion(int spanRows, int spanCols) {
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


void NewArch::GridLayout::resolveStructure(size_t templateRows, size_t templateCols) {
    grid = Grid{templateRows, templateCols};

    // place explicitly placed items
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

        auto [row, col] = grid.findEmptyRegion(spanRows, spanCols);

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