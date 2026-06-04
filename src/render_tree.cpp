#include "render_tree.hpp"
#include "hash_combine.hpp"
#include <algorithm>
#include <print>

namespace tree {
    using layout::FlexLayout;
    using layout::FlexResolver;
    using layout::GridResolver;
    using layout::MarginMetadata;
    using layout::SizeResolutionContext;
    using style::ClipUniform;
    using style::Unit;

    bool RenderTree::isFrameInfoChanged(const FrameInfo& frameInfo) const {
        return !lastFrameInfo.has_value()
            || lastFrameInfo->width != frameInfo.width
            || lastFrameInfo->height != frameInfo.height
            || lastFrameInfo->scale != frameInfo.scale;
    }

    void RenderTree::markDirty() {
        needsUpdate = true;
        pendingFrameBufferWrites = MaxOutstandingFrameCount;
        renderOrderDirty = true;
        if (auto root = getRoot()) {
            markSubtreeDirty(root, allPhaseDirtyBits());
        }
    }

    void RenderTree::markDirty(TreeNode* node, DirtyBits bits) {
        if (!node || bits == DirtyBits::None) return;

        needsUpdate = true;
        pendingFrameBufferWrites = MaxOutstandingFrameCount;

        if (hasDirty(bits, DirtyBits::PaintOrder)) {
            renderOrderDirty = true;
        }

        DirtyBits selfBits = bits;
        if (hasDirty(bits, DirtyBits::Measure | DirtyBits::Atomize | DirtyBits::Layout)) {
            selfBits |= DirtyBits::PostLayout | DirtyBits::Place | DirtyBits::Finalize;
        }
        if (hasDirty(bits, DirtyBits::PostLayout)) {
            selfBits |= DirtyBits::Place | DirtyBits::Finalize;
        }
        if (hasDirty(bits, DirtyBits::Place)) {
            selfBits |= DirtyBits::Finalize;
        }

        node->dirtySelf |= selfBits;
        node->dirtySubtree |= selfBits;

        for (auto* ancestor = node->parent; ancestor; ancestor = ancestor->parent) {
            ancestor->dirtySubtree |= selfBits;
        }

        if (hasDirty(bits, DirtyBits::PostLayout | DirtyBits::Place)) {
            markSubtreeDirty(node, selfBits & (DirtyBits::PostLayout | DirtyBits::Place | DirtyBits::Finalize));
        }

        if (hasDirty(bits, DirtyBits::Measure | DirtyBits::Atomize | DirtyBits::Layout)) {
            DirtyBits ancestorBits = DirtyBits::Layout | DirtyBits::PostLayout | DirtyBits::Place | DirtyBits::Finalize;
            for (auto* ancestor = node->parent; ancestor; ancestor = ancestor->parent) {
                ancestor->dirtySelf |= ancestorBits;
                ancestor->dirtySubtree |= ancestorBits | selfBits;
            }
        }

        if (hasDirty(bits, DirtyBits::PostLayout)) {
            DirtyBits ancestorBits = DirtyBits::PostLayout | DirtyBits::Place | DirtyBits::Finalize;
            for (auto* ancestor = node->parent; ancestor; ancestor = ancestor->parent) {
                ancestor->dirtySelf |= ancestorBits;
                ancestor->dirtySubtree |= ancestorBits | selfBits;
            }
        }
    }

    void RenderTree::markSubtreeDirty(TreeNode* node, DirtyBits bits) {
        if (!node || bits == DirtyBits::None) return;
        node->dirtySelf |= bits;
        node->dirtySubtree |= bits;
        for (auto& child : node->children) {
            markSubtreeDirty(child.get(), bits);
        }
    }

    void RenderTree::clearDirty(TreeNode* node) {
        if (!node) return;
        node->dirtySelf = DirtyBits::None;
        node->dirtySubtree = DirtyBits::None;
        for (auto& child : node->children) {
            clearDirty(child.get());
        }
    }

    bool RenderTree::subtreeHasDirty(TreeNode* node, DirtyBits bits) const {
        if (!node) return false;
        return hasDirty(node->dirtySelf | node->dirtySubtree, bits);
    }

    ConstraintsKey RenderTree::makeConstraintsKey(const Constraints& constraints,
                                                  simd_float2 extraOriginA,
                                                  simd_float2 extraOriginB) const {
        std::size_t hash = 0;
        hash_combine(hash, constraints.origin.x);
        hash_combine(hash, constraints.origin.y);
        hash_combine(hash, constraints.cursor.x);
        hash_combine(hash, constraints.cursor.y);
        hash_combine(hash, constraints.availableWidth);
        hash_combine(hash, constraints.availableHeight);
        hash_combine(hash, constraints.frameInfo.width);
        hash_combine(hash, constraints.frameInfo.height);
        hash_combine(hash, constraints.frameInfo.scale);
        hash_combine(hash, constraints.absoluteContainingBlock.origin.x);
        hash_combine(hash, constraints.absoluteContainingBlock.origin.y);
        hash_combine(hash, constraints.absoluteContainingBlock.width);
        hash_combine(hash, constraints.absoluteContainingBlock.height);
        hash_combine(hash, constraints.shrinkToFit);
        hash_combine(hash, constraints.lineFragments.size());
        hash_combine(hash, constraints.lineBoxes.size());

        for (auto& clip : constraints.clipUniforms) {
            hash_combine(hash, clip.rectCenter.x);
            hash_combine(hash, clip.rectCenter.y);
            hash_combine(hash, clip.halfExtent.x);
            hash_combine(hash, clip.halfExtent.y);
            hash_combine(hash, clip.cornerRadius.x);
            hash_combine(hash, clip.cornerRadius.y);
        }

        for (auto& fragment : constraints.lineFragments) {
            hash_combine(hash, fragment.width);
            hash_combine(hash, fragment.atomCount);
            hash_combine(hash, fragment.lineBoxIndex);
            hash_combine(hash, fragment.fragmentIndex);
        }
        for (auto& lineBox : constraints.lineBoxes) {
            hash_combine(hash, lineBox.fragmentCount);
            hash_combine(hash, lineBox.width);
            hash_combine(hash, lineBox.currentFragmentOffset);
            for (auto offset : lineBox.fragmentOffsets) {
                hash_combine(hash, offset);
            }
        }

        hash_combine(hash, extraOriginA.x);
        hash_combine(hash, extraOriginA.y);
        hash_combine(hash, extraOriginB.x);
        hash_combine(hash, extraOriginB.y);

        return ConstraintsKey{.value = hash};
    }

    bool RenderTree::shouldRecompute(TreeNode* node, DirtyBits bit, const ConstraintsKey& incomingKey) const {
        return hasDirty(node->dirtySelf, bit) ||
            !node->constraintsKey.has_value() ||
            *node->constraintsKey != incomingKey;
    }

    const std::vector<TreeNode*>& RenderTree::sortedRenderOrder() {
        if (!renderOrderDirty && !renderOrderCache.empty()) {
            debugCounters.renderOrderCacheHits++;
            return renderOrderCache;
        }

        debugCounters.renderOrderCacheMisses++;
        renderOrderCache = collectAllNodes(getRoot());
        std::sort(renderOrderCache.begin(), renderOrderCache.end(), [](TreeNode* a, TreeNode* b) {
            if (a->globalZIndex != b->globalZIndex) {
                return a->globalZIndex < b->globalZIndex;
            }
            return a->id < b->id;
        });
        renderOrderDirty = false;
        return renderOrderCache;
    }

    // I have a render cache, develop some sort of caching policy that makes these useful
    void RenderTree::update(const FrameInfo& frameInfo, uint64_t frameIndex) {
        debugCounters = {};
        bool frameInfoChanged = isFrameInfoChanged(frameInfo);
        if (frameInfoChanged) {
            pendingFrameBufferWrites = MaxOutstandingFrameCount;
            if (auto root = getRoot()) {
                markSubtreeDirty(root, allPhaseDirtyBits());
            }
            renderOrderDirty = true;
        }

        if (!needsUpdate && !frameInfoChanged && pendingFrameBufferWrites == 0) {
            return;
        }

        needsUpdate = false;
        lastFrameInfo = frameInfo;

        auto root = getRoot();
        if (!root) return;


        rootCursor = simd_float2{0,0};
        rootConstraints = Constraints{
            .origin = simd_float2{0,0},
            .cursor = rootCursor,
            .availableWidth = frameInfo.width,
            .availableHeight = frameInfo.height,
            .frameInfo = frameInfo,
            .absoluteContainingBlock = {
                .origin = {0, 0},
                .width = frameInfo.width,
                .height = frameInfo.height
            },
            .clipUniforms = {
                ClipUniform {
                    .rectCenter = {frameInfo.width * 0.5f, frameInfo.height * 0.5f},
                    .halfExtent = {frameInfo.width * 0.5f, frameInfo.height * 0.5f},
                    .cornerRadius = {0.0f, 0.0f}
                }
            },
        };

        // AHH APPLE CLANG DOESN'T SUPPORT EXECUTION POLICIES YET EXECUTE ME
        // Parallel::for_each(allNodes.begin(), allNodes.end(),
        //     [&](TreeNode* node) {
        //         node->measured = node->element->measure(rootConstraints);
        //     }
        // );

        if (subtreeHasDirty(root, DirtyBits::Measure) || !root->measured.has_value()) {
            measurePhase(root, rootConstraints);
        }
        if (subtreeHasDirty(root, DirtyBits::Atomize) || !root->atomized.has_value()) {
            atomizePhase(root, rootConstraints);
        }
    
        // Layout pass
        // precompute margin metadata + intents
        bool needsLayoutPass = subtreeHasDirty(root, DirtyBits::Layout) || !root->layout.has_value();
        if (needsLayoutPass && (subtreeHasDirty(root, DirtyBits::Layout) || !root->preLayout.has_value())) {
            preLayoutPhase(root, frameInfo, rootConstraints);
        }
        // initial layout pass
        if (needsLayoutPass) {
            layoutPhase(root, frameInfo, rootConstraints);
            root->calculateGlobalZIndex(0);
        }
        sortedRenderOrder();
        // postLayout: resolve global positions (serial, top-down) + reconcile atoms
        if (subtreeHasDirty(root, DirtyBits::PostLayout) || !root->layout.has_value()) {
            postLayoutPhase(root, frameInfo, rootConstraints, {0.0f, 0.0f}, {0.0f, 0.0f});
        }

        if (subtreeHasDirty(root, DirtyBits::Place) || !root->placed.has_value()) {
            placePhase(root, frameInfo, rootConstraints);
        }
        if (subtreeHasDirty(root, DirtyBits::Finalize) || !root->finalized.has_value()) {
            finalizePhase(root, rootConstraints);
        }

        if (pendingFrameBufferWrites > 0) {
            pendingFrameBufferWrites--;
        }
        if (pendingFrameBufferWrites == 0) {
            clearDirty(root);
        }

        if (debugDirtyPhases) {
            std::println(
                "dirty phases: measure {}/{} atomize {}/{} layout {}/{} post {}/{} place {}/{} finalize {}/{} order hit/miss {}/{}",
                debugCounters.measure.recomputed, debugCounters.measure.skipped,
                debugCounters.atomize.recomputed, debugCounters.atomize.skipped,
                debugCounters.layout.recomputed, debugCounters.layout.skipped,
                debugCounters.postLayout.recomputed, debugCounters.postLayout.skipped,
                debugCounters.place.recomputed, debugCounters.place.skipped,
                debugCounters.finalize.recomputed, debugCounters.finalize.skipped,
                debugCounters.renderOrderCacheHits, debugCounters.renderOrderCacheMisses
            );
        }
    }

    void RenderTree::render(MTL::RenderCommandEncoder* encoder) {
        auto& allNodes = sortedRenderOrder();
        
        // serially encoded; encoders are not thread safe
        for (auto node : allNodes) { 
            auto& finalized = node->finalized;
            node->element->encode(encoder, finalized);
        }
    }

    void RenderTree::measurePhase(TreeNode* node, Constraints& constraints) {
        auto key = makeConstraintsKey(constraints);
        bool recompute = shouldRecompute(node, DirtyBits::Measure, key);
        if (recompute) {
            auto measured = node->element->measure(constraints, node->shared);
            node->measured = measured;
            node->constraintsKey = key;
            node->dirtySelf |= DirtyBits::Atomize | DirtyBits::Layout | DirtyBits::PostLayout | DirtyBits::Place | DirtyBits::Finalize;
            debugCounters.measure.recomputed++;
        } else {
            debugCounters.measure.skipped++;
        }
        
        float paddingLeft = node->shared.paddingLeft.value_or(Size{}).resolveOr(constraints.availableWidth);
        float paddingTop = node->shared.paddingTop.value_or(Size{}).resolveOr(constraints.availableHeight);
        float paddingRight = node->shared.paddingRight.value_or(Size{}).resolveOr(constraints.availableWidth);
        float paddingBottom = node->shared.paddingBottom.value_or(Size{}).resolveOr(constraints.availableHeight);

        Constraints childConstraints {};

        // std::println("padidngLeft: {} paddingRight: {}", paddingLeft, paddingRight);
    
        childConstraints.availableWidth = node->measured->explicitWidth.value_or(constraints.availableWidth) - paddingLeft - paddingRight;
        childConstraints.availableHeight = node->measured->explicitHeight.value_or(constraints.availableHeight) - paddingTop - paddingBottom;
        
        // std::println("maxWidth: {}", childConstraints.availableWidth);

        for (auto& child : node->children) {
            measurePhase(child.get(), childConstraints);
        }
    }


    // consider safer way of accessing cache?
    void RenderTree::atomizePhase(TreeNode* node, Constraints& constraints) {
        assert(node->measured.has_value());

        auto key = makeConstraintsKey(constraints);
        bool recompute = shouldRecompute(node, DirtyBits::Atomize, key);
        if (recompute) {
            auto& measured  = *node->measured;
            auto& shared = node->shared;
            auto atomized = node->element->atomize(constraints, shared, measured);
            node->atomized = atomized;
            node->constraintsKey = key;
            node->dirtySelf |= DirtyBits::Layout | DirtyBits::PostLayout | DirtyBits::Place | DirtyBits::Finalize;
            debugCounters.atomize.recomputed++;
        } else {
            debugCounters.atomize.skipped++;
        }

        for (auto& child : node->children) {
            atomizePhase(child.get(), constraints);
        }
    }

    void buildCollapsedChains(
        TreeNode* node,
        std::unordered_map<ChainID, CollapsedChain>& chainMap,
        ChainID& nextChainId,
        CollapsedChain* collapsedTopChain,
        CollapsedChain* collapsedBottomChain
    )
    {
        // find first/last collapsable child
        TreeNode* firstInFlowCollapsableChild = nullptr;
        TreeNode* lastInFlowCollapsableChild = nullptr;

        for (auto& child : node->children) {
            auto position = child->getPosition();
            auto display = child->getDisplay();

            if ((position == Position::Static || position == Position::Relative) && display == Display::Block) {
                if (firstInFlowCollapsableChild == nullptr) firstInFlowCollapsableChild = child.get();
                lastInFlowCollapsableChild = child.get();
            }
        }

        // check if out of flow
        auto currPosition = node->getPosition();
        bool currOutOfFlow = currPosition == Position::Absolute || currPosition == Position::Fixed;

        if (currOutOfFlow) {
            firstInFlowCollapsableChild = nullptr;
            lastInFlowCollapsableChild = nullptr;
        }

        // skip continuation if padding defined
        bool nodeBlocksTopChain = node->getPaddingTop().has_value();
        bool nodeBlocksBottomChain = node->getPaddingBottom().has_value();

        if (nodeBlocksTopChain) {
            firstInFlowCollapsableChild = nullptr;
        }

        if (nodeBlocksBottomChain) {
            lastInFlowCollapsableChild = nullptr;
        }

        Size marginTop = node->getMarginTop();
        Size marginBottom = node->getMarginBottom();

        // check if has top chains; if not, create new ones (which will be propagated to first and last child)
        CollapsedChain newCollapsedTopChain;
        CollapsedChain newCollapsedBottomChain;
        CollapsedChain* propagatedTopChain = collapsedTopChain;
        CollapsedChain* propagatedBottomChain = collapsedBottomChain;

        if (!collapsedTopChain) {
            newCollapsedTopChain = {
                .id = nextChainId++,
                .root = node,
                .intent = marginTop,
                .depth = 1
            };

            propagatedTopChain = &newCollapsedTopChain;
        }else {
            if (marginTop.value > collapsedTopChain->intent.value) {
                collapsedTopChain->intent = marginTop;
            }
            
            collapsedTopChain->depth++;

            if (!firstInFlowCollapsableChild) {
                chainMap[collapsedTopChain->id] = *collapsedTopChain;
            }
        }

        if (!collapsedBottomChain) {
            newCollapsedBottomChain = {
                .id = nextChainId++,
                .root = node,
                .intent = marginBottom,
                .depth = 1
            };

            propagatedBottomChain = &newCollapsedBottomChain;
        }else {
            if (marginBottom.value > collapsedBottomChain->intent.value) {
                collapsedBottomChain->intent = marginBottom;
            }

            collapsedBottomChain->depth++;

            if (!lastInFlowCollapsableChild) {
                chainMap[collapsedBottomChain->id] = *collapsedBottomChain;
            }
        }

        for (auto& child : node->children) {
            auto rawChild = child.get();

            if (rawChild == firstInFlowCollapsableChild) {
                buildCollapsedChains(rawChild, chainMap, nextChainId, propagatedTopChain, nullptr);
            }else if (rawChild == lastInFlowCollapsableChild) {
                buildCollapsedChains(rawChild, chainMap, nextChainId, nullptr, propagatedBottomChain);
            }else {
                buildCollapsedChains(rawChild, chainMap, nextChainId, nullptr, nullptr);
            }
        }

        MarginMetadata marginMetadata {
            .topChainId = propagatedTopChain->id,
            .bottomChainId = propagatedBottomChain->id
        };

        node->preLayout = PreLayoutResult{};

        node->preLayout->marginMetadata = marginMetadata;
    }


    void RenderTree::preLayoutPhase(TreeNode* node, const FrameInfo& frameInfo, Constraints& constraints) {
        collapsedChainMap.clear();
        nextChainId = 0;

        buildCollapsedChains(node, collapsedChainMap, nextChainId, nullptr, nullptr);

        precomputeMargins(node, constraints, collapsedChainMap);
    }

    void RenderTree::layoutPhase(TreeNode* node, const FrameInfo& frameInfo, Constraints& constraints) {
        assert(node->measured.has_value());
        assert(node->atomized.has_value());
        assert(node->preLayout.has_value());

        auto key = makeConstraintsKey(constraints);
        debugCounters.layout.recomputed++;

        auto& measured = *node->measured;
        auto& atomized = *node->atomized;
        auto& prelayout = *node->preLayout;

        constraints.resolvedMargins = prelayout.resolvedMargins;

        // Re-resolve percent sizes from current constraints, but only in final layout
        // passes (shrinkToFit = false). During intermediate measurements (shrinkToFit = true),
        // maxWidth comes from an indefinite ancestor and would give wrong values.
        if (!constraints.shrinkToFit &&
            ((node->shared.width.has_value() && node->shared.width->unit == Unit::Percent) ||
             (node->shared.height.has_value() && node->shared.height->unit == Unit::Percent))
        ) {
            SizeResolutionContext sizeCtx {
                .position = node->shared.position,
                .parentConstraints = constraints,
                .top = node->shared.top,
                .right = node->shared.right,
                .bottom = node->shared.bottom,
                .left = node->shared.left,
                .requestedWidth = node->shared.width,
                .requestedHeight = node->shared.height,
                .availableWidth = constraints.availableWidth,
                .availableHeight = constraints.availableHeight
            };

            auto newSize = resolveSize(sizeCtx);
            
            if (node->shared.width.has_value() && node->shared.width->unit == Unit::Percent) {
                node->measured->explicitWidth = newSize.width;
            }
            if (node->shared.height.has_value() && node->shared.height->unit == Unit::Percent) {
                node->measured->explicitHeight = newSize.height;
            }
        }


        auto layout = node->element->layout(constraints, node->shared, measured, atomized);

        auto childConstraints = layout.childConstraints;
        float parentAvailableWidth  = childConstraints.availableWidth;
        float parentAvailableHeight = childConstraints.availableHeight;
        float originX         = childConstraints.origin.x;
        float originY         = childConstraints.origin.y;

        auto position = node->getPosition();
        if (position != Position::Static) {
            childConstraints.absoluteContainingBlock = {
                .origin = {0.0f, 0.0f},
                .width = layout.computedBox.width,
                .height = layout.computedBox.height
            };
        } else {
            childConstraints.absoluteContainingBlock = constraints.absoluteContainingBlock;
        }

        float minY = originY;
        float maxY = originY;
        float minX = originX;
        float maxX = originX;

        auto display = node->getDisplay();
        bool isNormalFlow = display != Display::Flex && display != Display::Grid;
        auto&& [childrenLineFragments, childrenLineBoxes] = buildInlineBoxes(node, childConstraints);
        childConstraints.lineBoxes = childrenLineBoxes;

        auto normalPass = [&](){
            for (uint64_t i = 0; i < node->children.size(); ++i) {
                auto& child = node->children[i];
                auto childAsPtr = child.get();

                childConstraints.lineFragments = childrenLineFragments[i];
                childConstraints.inheritedProperties = constraints.inheritedProperties;
                layoutPhase(childAsPtr, frameInfo, childConstraints);
                auto childLayout = *childAsPtr->layout;

                if (!childLayout.outOfFlow) {
                    childConstraints.cursor = childLayout.siblingCursor;
                    childConstraints.edgeIntent = childLayout.edgeIntent;
                
                    maxX = std::max(maxX, childLayout.computedBox.x + childLayout.computedBox.width);
                    maxY = std::max(maxY, childLayout.computedBox.y + childLayout.consumedHeight);
                }
            }
        };

        auto flexPass = [&]() {
            auto flexDirection = node->getFlexDirection();
            auto justifyContent = node->getJustifyContent();
            auto alignItems = node->getAlignItems();
            auto alignContentVal = node->getAlignContent();
            auto flexWrap = node->getFlexWrap();

            FlexLayout flexContext {flexDirection, justifyContent, alignItems, alignContentVal, flexWrap};
            flexContext.axis.applyDirection(constraints.inheritedProperties.direction);

            FlexResolver fr {
                *this, node, constraints, childConstraints, flexContext, frameInfo, parentAvailableWidth, parentAvailableHeight, minX, minY, maxX, maxY
            };

            fr.phaseAB();
            fr.phaseC();
            auto bounds = fr.phaseD();

            maxX = bounds.maxX;
            maxY = bounds.maxY;
        };

        auto gridPass = [&]() {
            GridResolver gr {
                *this, node, constraints, childConstraints, frameInfo, parentAvailableWidth, parentAvailableHeight, minX, minY, maxX, maxY
            };

            gr.phaseA();
            gr.phaseB();
            gr.phaseC();

            auto bounds = gr.phaseD();


            maxX = bounds.maxX;
            maxY = bounds.maxY;
        };

        if (display == Display::Flex) {
            flexPass();
        } else if (display == Display::Grid) {
            gridPass();
        } else if (isNormalFlow) {
            normalPass();
        }

        // percent/auto does not contribute to parent intrinsic size. But we don't use intrinsic sizes to compute this. huh
        
        // resize width/height of underspecified elements
        if (!measured.explicitWidth.has_value()) {
            if (position != Position::Static || constraints.shrinkToFit) {
                layout.computedBox.width = maxX - minX + layout.resolvedPadding.left + layout.resolvedPadding.right;
            }
        }

        float usedWidth = layout.computedBox.width;
        if (node->shared.maxWidth.has_value()) {
            usedWidth = std::min(usedWidth, node->shared.maxWidth->resolveOr(constraints.availableWidth, usedWidth));
        }
        if (node->shared.minWidth.has_value()) {
            usedWidth = std::max(usedWidth, node->shared.minWidth->resolveOr(constraints.availableWidth, usedWidth));
        }

        if (!measured.explicitHeight.has_value()) {
            layout.computedBox.height = maxY - minY + layout.resolvedPadding.top + layout.resolvedPadding.bottom;
            layout.consumedHeight = layout.computedBox.height;
        }

        float usedHeight = layout.computedBox.height;
        if (node->shared.maxHeight.has_value()) {
            usedHeight = std::min(usedHeight, node->shared.maxHeight->resolveOr(constraints.availableHeight, usedHeight));
        }
        if (node->shared.minHeight.has_value()) {
            usedHeight = std::max(usedHeight, node->shared.minHeight->resolveOr(constraints.availableHeight, usedHeight));
        }

        bool widthChanged = usedWidth != layout.computedBox.width;
        bool heightChanged = usedHeight != layout.computedBox.height;

        if (widthChanged || heightChanged) {
            Measured retryMeasured = measured;
            if (widthChanged) retryMeasured.explicitWidth = usedWidth;
            if (heightChanged) retryMeasured.explicitHeight = usedHeight;

            auto retryInput = layout::toLayoutInput(node->shared, retryMeasured);
            constraints.resolvedMargins = layout::LayoutEngine::resolveAutoMargins(
                retryInput,
                constraints.replacedAttributes,
                constraints.availableWidth,
                usedWidth
            );

            layout = node->element->layout(constraints, node->shared, retryMeasured, atomized);
            childConstraints = layout.childConstraints;
            parentAvailableWidth  = childConstraints.availableWidth;
            parentAvailableHeight = childConstraints.availableHeight;

            if (position != Position::Static) {
                childConstraints.absoluteContainingBlock = {
                    .origin = {0.0f, 0.0f},
                    .width = layout.computedBox.width,
                    .height = layout.computedBox.height
                };
            } else {
                childConstraints.absoluteContainingBlock = constraints.absoluteContainingBlock;
            }

            originX = childConstraints.origin.x;
            originY = childConstraints.origin.y;
            minY = maxY = originY;
            minX = maxX = originX;

            auto [retryChildrenLineFragments, retryChildrenLineBoxes] = buildInlineBoxes(node, childConstraints);
            childrenLineFragments = retryChildrenLineFragments;
            childrenLineBoxes = retryChildrenLineBoxes;
            childConstraints.lineBoxes = childrenLineBoxes;

            if (display == Display::Flex) {
                flexPass();
            } else if (display == Display::Grid) {
                gridPass();
            } else if (isNormalFlow) {
                normalPass();
            }

            if (widthChanged) {
                layout.computedBox.width = usedWidth;
            } else if (!measured.explicitWidth.has_value() || constraints.shrinkToFit) {
                layout.computedBox.width = maxX - minX + layout.resolvedPadding.left + layout.resolvedPadding.right;
            }

            if (heightChanged) {
                layout.computedBox.height = usedHeight;
            } else if (!measured.explicitHeight.has_value() || constraints.shrinkToFit) {
                layout.computedBox.height = maxY - minY + layout.resolvedPadding.top + layout.resolvedPadding.bottom;
                if (node->shared.maxHeight.has_value()) {
                    layout.computedBox.height = std::min(layout.computedBox.height, node->shared.maxHeight->resolveOr(constraints.availableHeight, layout.computedBox.height));
                }
                if (node->shared.minHeight.has_value()) {
                    layout.computedBox.height = std::max(layout.computedBox.height, node->shared.minHeight->resolveOr(constraints.availableHeight, layout.computedBox.height));
                }
            }

            if (!layout.outOfFlow) {
                layout.consumedHeight = layout.computedBox.height;
            }
        }

        // finalize layout of node
        layout.localComputedBox = layout.computedBox;
        layout.localAtomOffsets = layout.atomOffsets;
        node->layout = layout;
        node->constraintsKey = key;
        node->dirtySelf |= DirtyBits::PostLayout | DirtyBits::Place | DirtyBits::Finalize;
    }

    void RenderTree::postLayoutPhase(TreeNode* node, const FrameInfo& frameInfo, Constraints& constraints,
                                      simd_float2 parentGlobalOrigin, simd_float2 absBlockGlobalOrigin) {
        assert(node->measured.has_value());
        assert(node->atomized.has_value());
        assert(node->layout.has_value());

        auto key = makeConstraintsKey(constraints, parentGlobalOrigin, absBlockGlobalOrigin);
        bool recompute = shouldRecompute(node, DirtyBits::PostLayout, key);
        if (!recompute) {
            debugCounters.postLayout.skipped++;
            return;
        }
        debugCounters.postLayout.recomputed++;

        auto& layout = *node->layout;
        layout.computedBox = layout.localComputedBox;
        layout.atomOffsets = layout.localAtomOffsets;

        auto position = node->getPosition();

        auto& dp = layout.deferredPosition;
        if (dp.needsRightResolution) {
            float newX = dp.containingBlockWidth - dp.marginRight - layout.computedBox.width - dp.resolvedRight;
            float deltaX = newX - layout.computedBox.x;
            layout.computedBox.x = newX;
            for (auto& offset : layout.atomOffsets) {
                offset.x += deltaX;
            }
        }

        if (dp.needsBottomResolution) {
            float newY = dp.containingBlockHeight - dp.marginBottom - layout.computedBox.height - dp.resolvedBottom;
            float deltaY = newY - layout.computedBox.y;
            layout.computedBox.y = newY;
            for (auto& offset : layout.atomOffsets) {
                offset.y += deltaY;
            }
        }

        simd_float2 baseOrigin;
        if (position == Position::Fixed) {
            baseOrigin = {0.0f, 0.0f};
        } else if (position == Position::Absolute) {
            baseOrigin = absBlockGlobalOrigin;
        } else {
            baseOrigin = parentGlobalOrigin;
        }

        layout.computedBox.x += baseOrigin.x;
        layout.computedBox.y += baseOrigin.y;
        for (auto& offset : layout.atomOffsets) {
            offset.x += baseOrigin.x;
            offset.y += baseOrigin.y;
        }
        node->globalOffset = baseOrigin;
        layout.clipUniforms = constraints.clipUniforms;

        if (node->shared.overflow == Overflow::Scroll) {
            float viewportLeft = layout.computedBox.x;
            float viewportRight = layout.computedBox.x + layout.computedBox.width;
            float viewportTop = layout.computedBox.y;
            float viewportBottom = layout.computedBox.y + layout.computedBox.height;

            for (auto& clip : constraints.clipUniforms) {
                viewportLeft = std::max(viewportLeft, clip.rectCenter.x - clip.halfExtent.x);
                viewportRight = std::min(viewportRight, clip.rectCenter.x + clip.halfExtent.x);
                viewportTop = std::max(viewportTop, clip.rectCenter.y - clip.halfExtent.y);
                viewportBottom = std::min(viewportBottom, clip.rectCenter.y + clip.halfExtent.y);
            }

            viewportLeft += layout.resolvedPadding.left;
            viewportRight -= layout.resolvedPadding.right;
            viewportTop += layout.resolvedPadding.top;
            viewportBottom -= layout.resolvedPadding.bottom;

            node->scrollViewportSize = {
                std::max(0.0f, viewportRight - viewportLeft),
                std::max(0.0f, viewportBottom - viewportTop)
            };
        }

        node->atomized = node->element->postLayout(constraints, node->shared, *node->measured,
                                                    *node->atomized, layout);

        simd_float2 currContentOrigin = {
            layout.computedBox.x + layout.resolvedPadding.left,
            layout.computedBox.y + layout.resolvedPadding.top
        };

        if (node->shared.overflow == Overflow::Scroll) {
            currContentOrigin.x -= node->scrollOffset.x;
            currContentOrigin.y -= node->scrollOffset.y;
        }

        simd_float2 childAbsBlockOrigin = absBlockGlobalOrigin;
        if (position != Position::Static) {
            childAbsBlockOrigin = currContentOrigin;
        }

        auto childConstraints = constraints;
        if (node->shared.overflow != Overflow::Visible) {
            float cornerRadius = node->shared.cornerRadius.resolveOr(
                std::min(layout.computedBox.width, layout.computedBox.height)
            );

            simd_float2 halfExtent {
                layout.computedBox.width * 0.5f,
                layout.computedBox.height * 0.5f
            };

            childConstraints.clipUniforms.push_back({
                .rectCenter = {
                    layout.computedBox.x + halfExtent.x,
                    layout.computedBox.y + halfExtent.y
                },
                .halfExtent = halfExtent,
                .cornerRadius = {cornerRadius, cornerRadius}
            });

        }

        for (auto& child : node->children) {
            postLayoutPhase(child.get(), frameInfo, childConstraints,
                           currContentOrigin, childAbsBlockOrigin);
        }

        if (node->shared.overflow == Overflow::Scroll) {
            simd_float2 contentSize {0.0f, 0.0f};
            std::function<void(TreeNode*)> includeChildOverflow;
            includeChildOverflow = [&](TreeNode* child) {
                if (!child->layout.has_value() || child->layout->outOfFlow) return;
                auto& childBox = child->layout->computedBox;
                contentSize.x = std::max(contentSize.x, childBox.x + childBox.width - currContentOrigin.x);
                contentSize.y = std::max(contentSize.y, childBox.y + childBox.height - currContentOrigin.y);

                if (child->shared.overflow != Overflow::Visible) return;
                for (auto& grandchild : child->children) {
                    includeChildOverflow(grandchild.get());
                }
            };

            for (auto& child : node->children) {
                includeChildOverflow(child.get());
            }
            node->scrollContentSize = contentSize;
        }

        node->constraintsKey = key;
        node->dirtySelf |= DirtyBits::Place | DirtyBits::Finalize;
    }

    void RenderTree::placePhase(TreeNode* node, const FrameInfo& frameInfo, Constraints& constraints) {
        assert(node->measured.has_value());
        assert(node->atomized.has_value());
        assert(node->layout.has_value());

        auto key = makeConstraintsKey(constraints);
        bool recompute = shouldRecompute(node, DirtyBits::Place, key);
        if (recompute) {
            auto& measured = *node->measured;
            auto& atomized = *node->atomized;
            auto& layout = *node->layout;

            auto placed = node->element->place(constraints, node->shared, measured, atomized, layout);
            node->placed = placed;
            node->constraintsKey = key;
            node->dirtySelf |= DirtyBits::Finalize;
            debugCounters.place.recomputed++;
        } else {
            debugCounters.place.skipped++;
        }

        for (auto& child : node->children) {
            placePhase(child.get(), frameInfo, constraints);
        }
    }

    void RenderTree::finalizePhase(TreeNode* node, Constraints& constraints) {
        // I thought runtime checks for private methods were stupid, but I also am not superhuman and LLMs can hallucinate,
        // so debug asserts never hurt
        assert(node->measured.has_value());
        assert(node->atomized.has_value());
        assert(node->layout.has_value());
        assert(node->placed.has_value());

        auto key = makeConstraintsKey(constraints);
        bool recompute = shouldRecompute(node, DirtyBits::Finalize, key);
        if (recompute) {
            auto& measured =  *node->measured;
            auto& atomized = *node->atomized;
            auto& layout = *node->layout;
            auto& placed = *node->placed;
            auto finalized = node->element->finalize(constraints, node->shared, measured, atomized, layout, placed);
            node->finalized = finalized;
            node->constraintsKey = key;
            debugCounters.finalize.recomputed++;
        } else {
            debugCounters.finalize.skipped++;
        }

        for (auto& child : node->children) {
            finalizePhase(child.get(), constraints);
        }
    }

    TreeNode* RenderTree::hitTestRecursive(TreeNode* node, simd_float2 point) {
        if (!node) return nullptr;
        
        std::vector<TreeNode*> sortedChildren;
        for (auto& child : node->children) {
            sortedChildren.push_back(child.get());
        }
        std::sort(sortedChildren.begin(), sortedChildren.end(), 
            [](TreeNode* a, TreeNode* b) {
                if (a->globalZIndex != b->globalZIndex) {
                    return a->globalZIndex > b->globalZIndex;
                }
                return a->id > b->id;
            });
        
        for (auto* child : sortedChildren) {
            TreeNode* hit = hitTestRecursive(child, point);
            if (hit) return hit;
        }
        
        if (node->contains(point)) {
            return node;
        }
        
        return nullptr;
    }
}
