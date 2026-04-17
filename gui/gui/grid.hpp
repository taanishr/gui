#include "element.hpp"
#include <optional>
#include <vector>

namespace NewArch {
    struct ItemPlacement { 
        std::optional<int> colStart;
        std::optional<int> colEnd;
        std::optional<int> rowStart;
        std::optional<int> rowEnd; 

        bool colNeedsResolution() {
            return !(colStart.has_value() && colEnd.has_value());
        }   

        bool rowNeedsResolution() {
            return !(rowStart.has_value() && rowEnd.has_value());
        }
    };

    enum class GridDirection {
        Col,
        Row
    };

    struct Grid {
        std::vector<std::vector<uint8_t>> occupied; // [row][col]
        size_t numRows;
        size_t numCols;
        GridDirection majorAxis;

        int cursorMajor {};
        int cursorMinor {};

        Grid(size_t rows, size_t cols, GridDirection major = GridDirection::Row);

        void mark(int row, int col);
        bool regionFree(int row, int col, int spanRows, int spanCols) const;
        void growMajor(int needed);
        void advanceCursor(int spanMinor);

        std::pair<int, int> findSpace(int spanRows, int spanCols);

        int majorSize() const;
        int minorSize() const;
    };

    struct Track {
        float size;
        float offset;
    };
    
    struct GridLayout {
        std::vector<ItemPlacement> items;
        Grid grid {0, 0};
        std::vector<Track> rowTracks;
        std::vector<Track> colTracks;

        void addChild(NewArch::TreeNode* node);
        
        // helpers
        void resolveStructure(size_t templateRows, size_t templateCols);
        std::vector<Track> resolveTracks(std::vector<Size>& templateTracks, std::vector<float> itemSizes, float available, float gap, bool isMajorAxis);

        void resolve(size_t numRows, size_t numCols, std::vector<Size>& templateRows, std::vector<Size>& templateCols);
    };

    struct GridResolver {
        // copy from flex layout; responsible for calculating intrinsic sizes
        void phaseA(); 
        void phaseB();

        // here, we resolve the explicit sizes and collect grid metadata (i.e. track sizes) by calling addChild
        void phaseC(); 
    };
}

// auto& templateCols = getGridTemplateColumns(node);
// auto& templateRows = getGridTemplateRows(node);
// float availableWidth = measured.explicitWidth.value_or(constraints.maxWidth);
// float availableHeight = measured.explicitHeight.value_or(constraints.maxHeight);
// float colGap = getGridColumnGap(node).resolveOr(availableWidth, 0);
// float rowGap = getGridRowGap(node).resolveOr(availableHeight, 0);
// auto gridAlignItems = getAlignItems(node);

// size_t numCols = templateCols.size();
// // mutable copy of row templates (may grow with implicit rows)
// std::vector<Size> rowDefs(templateRows.begin(), templateRows.end());
// size_t numRows = rowDefs.size();

// float childMaxWidth = measured.explicitWidth.value_or(constraints.maxWidth);

// // Phase A+B: resolve intrinsic sizes for indefinite children
// auto isXIndefinite = [&](TreeNode* child) {
// return !measured.explicitWidth.has_value() &&
//     child->shared.width.has_value() && child->shared.width->unit == Unit::Percent;
// };
// auto isYIndefinite = [&](TreeNode* child) {
// return !measured.explicitHeight.has_value() &&
//     child->shared.height.has_value() && child->shared.height->unit == Unit::Percent;
// };

// bool hasIndefiniteChild = false;
// for (auto& child : node->children) {
// if (isXIndefinite(child.get()) || isYIndefinite(child.get())) {
//     hasIndefiniteChild = true;
//     break;
// }
// }

// if (hasIndefiniteChild) {
// float maxIntrinsicX = minX;
// float maxIntrinsicY = minY;

// // Phase A: lay out all children; indefinite ones get shrink-to-fit
// for (uint64_t i = 0; i < node->children.size(); ++i) {
//     auto childAsPtr = node->children[i].get();
//     bool xIndef = isXIndefinite(childAsPtr);
//     bool yIndef = isYIndefinite(childAsPtr);

//     std::optional<float> savedWidth, savedHeight;
//     if (xIndef) {
//         savedWidth = childAsPtr->measured->explicitWidth;
//         childAsPtr->measured->explicitWidth = std::nullopt;
//     }
//     if (yIndef) {
//         savedHeight = childAsPtr->measured->explicitHeight;
//         childAsPtr->measured->explicitHeight = std::nullopt;
//     }

//     auto&& [frags, boxes] = buildIsolatedInlineBoxes(childAsPtr, childMaxWidth);
//     childConstraints.lineFragments = frags;
//     childConstraints.lineBoxes = boxes;
//     childConstraints.maxWidth = childMaxWidth;
//     childConstraints.inheritedProperties = constraints.inheritedProperties;
//     bool isIndef = xIndef || yIndef;
//     bool savedShrink = childConstraints.shrinkToFit;
//     if (isIndef) childConstraints.shrinkToFit = true;

//     layoutPhase(childAsPtr, frameInfo, childConstraints);
//     auto& childLayout = *childAsPtr->layout;

//     maxIntrinsicX = std::max(maxIntrinsicX, childLayout.computedBox.x + childLayout.computedBox.width);
//     maxIntrinsicY = std::max(maxIntrinsicY, childLayout.computedBox.y + childLayout.consumedHeight);

//     if (xIndef) childAsPtr->measured->explicitWidth = savedWidth;
//     if (yIndef) childAsPtr->measured->explicitHeight = savedHeight;
//     childConstraints.shrinkToFit = savedShrink;
// }

// // Phase B: lay out indefinite children with corrected sizes.
// // Save/restore measured so modifications don't leak to subsequent passes.
// for (uint64_t i = 0; i < node->children.size(); ++i) {
//     auto childAsPtr = node->children[i].get();
//     bool xIndef = isXIndefinite(childAsPtr);
//     bool yIndef = isYIndefinite(childAsPtr);
//     if (!xIndef && !yIndef) continue;

//     if (xIndef) childAsPtr->measured->explicitWidth = maxIntrinsicX - minX;
//     if (yIndef) childAsPtr->measured->explicitHeight = maxIntrinsicY - minY;

//     auto&& [frags, boxes] = buildIsolatedInlineBoxes(childAsPtr, childMaxWidth);
//     childConstraints.lineFragments = frags;
//     childConstraints.lineBoxes = boxes;
//     childConstraints.maxWidth = childMaxWidth;
//     childConstraints.inheritedProperties = constraints.inheritedProperties;
//     layoutPhase(childAsPtr, frameInfo, childConstraints);
// }
// }

// // Phase C1: resolve item placements
// struct ItemPlacement { size_t childIdx; int c0, c1, r0, r1; };
// std::vector<ItemPlacement> items;
// std::vector<std::vector<bool>> occupied(numRows, std::vector<bool>(numCols, false));
// int autoCol = 0, autoRow = 0;

// for (size_t i = 0; i < node->children.size(); ++i) {
// auto childAsPtr = node->children[i].get();
// auto childPos = getPosition(childAsPtr);
// if (childPos == Position::Absolute || childPos == Position::Fixed) continue;

// auto gp = getGridPlacement(childAsPtr);
// int c0, c1, r0, r1;

// if (gp.colStart != 0) {
//     c0 = gp.colStart - 1;
//     c1 = (gp.colEnd != 0) ? gp.colEnd - 1 : c0 + 1;
// } else { c0 = -1; c1 = -1; }

// if (gp.rowStart != 0) {
//     r0 = gp.rowStart - 1;
//     r1 = (gp.rowEnd != 0) ? gp.rowEnd - 1 : r0 + 1;
// } else { r0 = -1; r1 = -1; }

// // Auto-placement for unplaced axes
// if (c0 == -1 || r0 == -1) {
//     int spanCols = (c0 != -1) ? (c1 - c0) : 1;
//     int spanRows = (r0 != -1) ? (r1 - r0) : 1;

//     while (true) {
//         if (autoCol + spanCols > (int)numCols) { autoCol = 0; autoRow++; }
//         if (autoRow + spanRows > (int)numRows) {
//             numRows = autoRow + spanRows;
//             rowDefs.resize(numRows, Size::autoSize());
//             occupied.resize(numRows, std::vector<bool>(numCols, false));
//         }
//         bool free = true;
//         for (int r = autoRow; r < autoRow + spanRows && free; ++r)
//             for (int c = autoCol; c < autoCol + spanCols && free; ++c)
//                 if (occupied[r][c]) free = false;
//         if (free) break;
//         autoCol++;
//     }
//     if (c0 == -1) { c0 = autoCol; c1 = autoCol + spanCols; }
//     if (r0 == -1) { r0 = autoRow; r1 = autoRow + spanRows; }
//     autoCol = c1;
//     if (autoCol >= (int)numCols) { autoCol = 0; autoRow++; }
// }

// // Mark occupied
// for (int r = r0; r < r1 && r < (int)numRows; ++r)
//     for (int c = c0; c < c1 && c < (int)numCols; ++c)
//         occupied[r][c] = true;

// items.push_back({i, c0, c1, r0, r1});
// }

// // Phase C2: initial child layout (for auto track sizing)
// for (auto& item : items) {
// auto childAsPtr = node->children[item.childIdx].get();
// if (!childAsPtr->layout.has_value() || !hasIndefiniteChild) {
//     auto&& [frags, boxes] = buildIsolatedInlineBoxes(childAsPtr, childMaxWidth);
//     childConstraints.lineFragments = frags;
//     childConstraints.lineBoxes = boxes;
//     childConstraints.maxWidth = childMaxWidth;
//     childConstraints.inheritedProperties = constraints.inheritedProperties;
//     layoutPhase(childAsPtr, frameInfo, childConstraints);
// }
// }

// // Phase C3: track sizing
// struct TrackInfo { float offset; float size; };

// // make this not a lambda??? should be in its own data structure in my refactor
// auto resolveTracks = [&](const std::vector<Size>& defs, float available, float gap,
//                         bool isCol) -> std::vector<TrackInfo> {
// size_t n = defs.size();
// if (n == 0) return {};
// float totalGap = (n > 1) ? gap * (float)(n - 1) : 0;
// float usable = available - totalGap;

// std::vector<float> sizes(n, 0);
// float fixedTotal = 0;
// float frTotal = 0;

// // Pass 1: fixed tracks
// for (size_t t = 0; t < n; ++t) {
//     if (defs[t].isFr()) {
//         frTotal += defs[t].value;
//     } else if (!defs[t].isAuto()) {
//         sizes[t] = defs[t].resolveOr(available, 0);
//         fixedTotal += sizes[t];
//     }
// }

// // Pass 2: auto tracks (max child natural size among single-track items)
// float autoTotal = 0;
// for (size_t t = 0; t < n; ++t) {
//     if (!defs[t].isAuto()) continue;
//     float maxChildSize = 0;
//     for (auto& item : items) {
//         int s = isCol ? item.c0 : item.r0;
//         int e = isCol ? item.c1 : item.r1;
//         if (s == (int)t && e == s + 1) {
//             auto& cl = *node->children[item.childIdx]->layout;
//             float sz = isCol ? cl.computedBox.width : cl.computedBox.height;
//             maxChildSize = std::max(maxChildSize, sz);
//         }
//     }
//     sizes[t] = maxChildSize;
//     autoTotal += maxChildSize;
// }

// // Pass 3: fr tracks
// float remaining = std::max(0.0f, usable - fixedTotal - autoTotal);
// if (frTotal > 0) {
//     for (size_t t = 0; t < n; ++t) {
//         if (defs[t].isFr()) {
//             sizes[t] = (defs[t].value / frTotal) * remaining;
//         }
//     }
// }

// // Compute offsets
// std::vector<TrackInfo> tracks(n);
// float offset = 0;
// for (size_t t = 0; t < n; ++t) {
//     tracks[t] = {offset, sizes[t]};
//     offset += sizes[t] + gap;
// }
// return tracks;
// };

// float contentWidth = parentMaxWidth;
// float contentHeight = parentMaxHeight;
// auto colTracks = resolveTracks(templateCols, contentWidth, colGap, true);
// auto rowTracks = resolveTracks(rowDefs, contentHeight, rowGap, false);

// // Phase D: apply placements and re-layout
// for (auto& item : items) {
// auto childAsPtr = node->children[item.childIdx].get();

// float cellX = colTracks[item.c0].offset;
// float cellY = rowTracks[item.r0].offset;
// float cellW = colTracks[item.c1 - 1].offset + colTracks[item.c1 - 1].size - cellX;
// float cellH = rowTracks[item.r1 - 1].offset + rowTracks[item.r1 - 1].size - cellY;

// auto&& [frags, boxes] = buildIsolatedInlineBoxes(childAsPtr, cellW);
// childConstraints.lineFragments = frags;
// childConstraints.lineBoxes = boxes;
// childConstraints.maxWidth = cellW;
// childConstraints.maxHeight = cellH;
// childConstraints.origin.x = cellX;
// childConstraints.cursor.y = cellY;
// childConstraints.inheritedProperties = constraints.inheritedProperties;
// childConstraints.shrinkToFit = false;

// // Resolve alignment
// AlignItems effectiveAlign = gridAlignItems;
// auto selfAlign = getAlignSelf(childAsPtr);
// if (selfAlign != AlignSelf::Auto) {
//     switch (selfAlign) {
//         case AlignSelf::Stretch:   effectiveAlign = AlignItems::Stretch; break;
//         case AlignSelf::FlexStart: effectiveAlign = AlignItems::FlexStart; break;
//         case AlignSelf::FlexEnd:   effectiveAlign = AlignItems::FlexEnd; break;
//         case AlignSelf::Center:    effectiveAlign = AlignItems::Center; break;
//         default: break;
//     }
// }

// // Stretch: override measured size if child has no explicit size
// if (effectiveAlign == AlignItems::Stretch) {
//     if (!childAsPtr->measured->explicitWidth.has_value())
//         childAsPtr->measured->explicitWidth = cellW;
//     if (!childAsPtr->measured->explicitHeight.has_value())
//         childAsPtr->measured->explicitHeight = cellH;
// } else {
//     childConstraints.shrinkToFit = true;
// }

// layoutPhase(childAsPtr, frameInfo, childConstraints);
// auto& childLayout = *childAsPtr->layout;

// // Non-stretch alignment: offset within cell
// if (effectiveAlign == AlignItems::Center) {
//     float dx = (cellW - childLayout.computedBox.width) / 2.0f;
//     float dy = (cellH - childLayout.computedBox.height) / 2.0f;
//     childLayout.computedBox.x += dx;
//     childLayout.computedBox.y += dy;
// } else if (effectiveAlign == AlignItems::FlexEnd) {
//     float dx = cellW - childLayout.computedBox.width;
//     float dy = cellH - childLayout.computedBox.height;
//     childLayout.computedBox.x += dx;
//     childLayout.computedBox.y += dy;
// }

// maxX = std::max(maxX, childLayout.computedBox.x + childLayout.computedBox.width);
// maxY = std::max(maxY, childLayout.computedBox.y + childLayout.consumedHeight);