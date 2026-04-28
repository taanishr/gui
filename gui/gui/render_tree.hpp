#pragma once

#include "element.hpp"
#include "flex.hpp"
#include "grid.hpp"
#include "renderer_constants.hpp"

namespace NewArch {
    struct LayoutSnapshotNode {
        uint64_t nodeId{};
        uint64_t parentId{};
        uint64_t zIndex{};
        float x{};
        float y{};
        float width{};
        float height{};
        float consumedHeight{};
        simd_float2 globalOffset{0.0f, 0.0f};
    };

    struct LayoutSnapshot {
        uint64_t generation{};
        FrameInfo frameInfo{};
        std::vector<LayoutSnapshotNode> nodes;
    };

    struct RenderTree {
        template<ElementType E, typename P>
            requires ProcessorType<P, typename E::StorageType, typename E::DescriptorType, typename E::UniformsType>
        TreeNode* createRoot(UIContext& ctx, E elem, P& processor) {
            elementTree = std::make_unique<TreeNode>(ctx, std::move(elem), processor);
            return elementTree.get();
        }
        
        TreeNode* getRoot() { return elementTree.get(); }
        
        void update(const FrameInfo& frameInfo, uint64_t frameIndex);
        void render(MTL::RenderCommandEncoder* encoder); 
        void markDirty();
        const LayoutSnapshot& getLayoutSnapshot() const { return currentLayoutSnapshot; }
        
        TreeNode* hitTestRecursive(TreeNode* node, simd_float2 point);
        

        void measurePhase(TreeNode* node, Constraints& constraints);
        void atomizePhase(TreeNode* node, Constraints& constraints);

        void preLayoutPhase(TreeNode* node, const FrameInfo& frameInfo, Constraints& constraints);

        
        void layoutPhase(TreeNode* node, const FrameInfo& frameInfo, Constraints& constraints);
        void postLayoutPhase(TreeNode* node, const FrameInfo& frameInfo, Constraints& constraints,
                             simd_float2 parentGlobalOrigin, simd_float2 absBlockGlobalOrigin);

        void placePhase(TreeNode* node, const FrameInfo& frameInfo, Constraints& constraints);
        void finalizePhase(TreeNode* node, Constraints& constraints);
    private:
        bool isFrameInfoChanged(const FrameInfo& frameInfo) const;
        void rebuildLayoutSnapshot(const FrameInfo& frameInfo, const std::vector<TreeNode*>& allNodes);

        bool needsUpdate{true};
        uint64_t pendingWarmupFrames{MaxOutstandingFrameCount};
        std::optional<FrameInfo> lastFrameInfo;
        uint64_t layoutSnapshotGeneration{0};
        LayoutSnapshot currentLayoutSnapshot;

        Constraints rootConstraints; 
        simd_float2 rootCursor;
        std::unordered_map<ChainID, CollapsedChain> collapsedChainMap;
        ChainID nextChainId = 0;

        std::unique_ptr<TreeNode> elementTree;
        LayoutEngine layoutEngine;
    };
}
