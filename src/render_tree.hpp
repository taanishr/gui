#pragma once

#include "element.hpp"
#include "flex.hpp"
#include "grid.hpp"
#include "renderer_constants.hpp"

namespace tree {
    using elements::ElementType;
    using elements::ProcessorType;
    using layout::ChainID;
    using layout::Constraints;
    using layout::LayoutEngine;
    using runtime::UIContext;

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
        void markDirty(TreeNode* node, DirtyBits bits);
  
        TreeNode* hitTestRecursive(TreeNode* node, simd_float2 point);
        std::vector<TreeNode*> hitTestAll(simd_float2 point);
        

        void measurePhase(TreeNode* node, Constraints& constraints);
        Result<void> atomizePhase(
            TreeNode* node,
            Constraints& constraints
        );

        void preLayoutPhase(TreeNode* node, const FrameInfo& frameInfo, Constraints& constraints);

        
        layout::LayoutOutput layoutPhase(
            TreeNode* node,
            const FrameInfo& frameInfo,
            Constraints constraints,
            layout::Measured measured
        );
        layout::LayoutOutput speculateLayout(
            TreeNode* node,
            const FrameInfo& frameInfo,
            Constraints constraints,
            layout::Measured measured
        );
        void postLayoutPhase(TreeNode* node, const FrameInfo& frameInfo, Constraints& constraints,
                             simd_float2 parentGlobalOrigin, simd_float2 absBlockGlobalOrigin);

        void placePhase(TreeNode* node, const FrameInfo& frameInfo, Constraints& constraints);
        void finalizePhase(TreeNode* node, Constraints& constraints);
    private:
        layout::LayoutOutput layoutRecursive(
            TreeNode* node,
            const FrameInfo& frameInfo,
            Constraints constraints,
            layout::Measured measured,
            bool mutate
        );

        bool isFrameInfoChanged(const FrameInfo& frameInfo) const;
        ConstraintsKey makeConstraintsKey(const Constraints& constraints,
                                          simd_float2 extraOriginA = {0.0f, 0.0f},
                                          simd_float2 extraOriginB = {0.0f, 0.0f}) const;
        bool shouldRecompute(TreeNode* node, DirtyBits bit, const ConstraintsKey& incomingKey) const;
        void markSubtreeDirty(TreeNode* node, DirtyBits bits);
        void clearDirty(TreeNode* node);
        bool subtreeHasDirty(TreeNode* node, DirtyBits bits) const;
        const std::vector<TreeNode*>& sortedRenderOrder();

        bool needsUpdate{true};
        // Retain dirty bits briefly after a mutation so every FrameBufferedBuffer slot
        // receives the new retained data before the node becomes clean.
        uint64_t pendingFrameBufferWrites{MaxOutstandingFrameCount};
        std::optional<FrameInfo> lastFrameInfo;
        uint64_t layoutGeneration{0};
        bool renderOrderDirty{true};
        std::vector<TreeNode*> renderOrderCache;


        Constraints rootConstraints; 
        simd_float2 rootCursor;
        std::unordered_map<ChainID, CollapsedChain> collapsedChainMap;
        ChainID nextChainId = 0;

        std::unique_ptr<TreeNode> elementTree;
        LayoutEngine layoutEngine;
    };
}
