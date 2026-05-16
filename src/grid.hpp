#pragma once

#include "element.hpp"
#include <optional>
#include <vector>

namespace tree {
    struct RenderTree;
}

namespace layout {
    using style::AlignItems;
    using style::Size;
    using tree::RenderTree;
    using tree::TreeNode;

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
        float offset;
        float size;
    };

    struct GridLayout {
        std::vector<ItemPlacement> items;
        Grid grid {0, 0};
        std::vector<Track> rowTracks;
        std::vector<Track> colTracks;

        void addChild(TreeNode* node);

        // helpers
        void resolveStructure(size_t templateRows, size_t templateCols);
        std::vector<Track> resolveTracks(std::vector<Size>& templateTracks, std::vector<float> itemSizes, float available, float gap, bool isCol);

        void resolve(size_t numRows, size_t numCols,
            const std::vector<Size>& templateRows, const std::vector<Size>& templateCols,
            float availableWidth, float availableHeight,
            float colGap, float rowGap,
            std::vector<float> itemWidths, std::vector<float> itemHeights);
    };

    struct GridResolver {
        RenderTree&       tree;
        TreeNode*         node;
        Constraints&      parentConstraints;
        Constraints       childConstraints;
        GridLayout        gridLayout;
        AlignItems        alignItems;
        const FrameInfo&  frameInfo;
        float             childMaxWidth;
        float             parentMaxWidth;
        float             parentMaxHeight;

        float minX;
        float minY;
        float maxX;
        float maxY;

        float maxIntrinsicX = 0;
        float maxIntrinsicY = 0;

        bool hasIndefiniteChild = false;

        std::vector<size_t> inFlowIndices;

        struct Bounds {
            float maxX;
            float maxY;
        };

        GridResolver(RenderTree& tree, TreeNode* node, Constraints& parentConstraints,
                     Constraints childConstraints, const FrameInfo& frameInfo,
                     float parentMaxWidth, float parentMaxHeight,
                     float minX, float minY, float maxX, float maxY);

        bool isXIndefinite(TreeNode* child) const;
        bool isYIndefinite(TreeNode* child) const;
        void prepareChildConstraints(TreeNode* child);

        void phaseA();
        void phaseB();
        void phaseC();
        Bounds phaseD();
    };
}
