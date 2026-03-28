#include "element.hpp"
#include "fragment_types.hpp"
#include "new_arch.hpp"
#include "printers.hpp"
#include <algorithm>
#include <any>
#include <cstdint>
#include <memory>
#include <print>
#include <ranges>
#include <simd/vector_types.h>
#include <unordered_map>
#include <ranges>


namespace NewArch {
    ElementBase::~ElementBase() {};

    // Helper functions — shared properties read directly from TreeNode::shared
    Position getPosition(TreeNode* node) { return node->shared.position; }
    Display getDisplay(TreeNode* node) { return node->shared.display; }
    Size getMarginTop(TreeNode* node) { return node->shared.marginTop.value_or(node->shared.margin); }
    Size getMarginBottom(TreeNode* node) { return node->shared.marginBottom.value_or(node->shared.margin); }
    Size getMarginLeft(TreeNode* node) { return node->shared.marginLeft.value_or(node->shared.margin); }
    Size getMarginRight(TreeNode* node) { return node->shared.marginRight.value_or(node->shared.margin); }
    Size getMargin(TreeNode* node) { return node->shared.margin; }
    std::optional<Size> getPaddingTop(TreeNode* node) { return node->shared.paddingTop; }
    std::optional<Size> getPaddingBottom(TreeNode* node) { return node->shared.paddingBottom; }
    Size getFlexGrow(TreeNode* node) { return node->shared.flexGrow; }
    Size getFlexShrink(TreeNode* node) { return node->shared.flexShrink; }
    FlexDirection getFlexDirection(TreeNode* node) { return node->shared.flexDirection; }
    JustifyContent getJustifyContent(TreeNode* node) { return node->shared.justifyContent; }
    AlignItems getAlignItems(TreeNode* node) { return node->shared.alignItems; }
    Size getFlexGap(TreeNode* node) { return node->shared.flexGap; }
    FlexWrap getFlexWrap(TreeNode* node) { return node->shared.flexWrap; }
    AlignContent getAlignContent(TreeNode* node) { return node->shared.alignContent; }
    AlignSelf getAlignSelf(TreeNode* node) { return node->shared.alignSelf; }

    // Element-specific requests still use the request system
    std::optional<std::string> getText(TreeNode* node) {
        std::any request{DescriptorPayload{GetField{.name = "text"}}};
        auto resp = node->element->request(RequestTarget::Descriptor, request);
        if (resp.has_value()) {
            return std::any_cast<std::string>(resp);
        }
        return std::nullopt;
    }

    

    uint64_t TreeNode::nextId = 0;

    std::vector<TreeNode*> collectAllNodes(TreeNode* root) {
        std::vector<TreeNode*> nodes;
        std::function<void(TreeNode*)> collect = [&](TreeNode* node) {
            if (!node) return;
            nodes.push_back(node);
            for (auto& child : node->children) {
                collect(child.get());
            }
        };
        collect(root);
        return nodes;
    }
    

    // I have a render cache, develop some sort of caching policy that makes these useful
    void RenderTree::update(const FrameInfo& frameInfo) {
        auto root = getRoot();
        auto allNodes = collectAllNodes(root);


        rootCursor = simd_float2{0,0};
        rootConstraints = Constraints{
            .origin = simd_float2{0,0},
            .cursor = rootCursor,
            .maxWidth = frameInfo.width,
            .maxHeight = frameInfo.height,
            .frameInfo = frameInfo,
            .absoluteContainingBlock = {
                .origin = {0, 0},
                .width = frameInfo.width,
                .height = frameInfo.height
            },
        };

        // AHH APPLE CLANG DOESN'T SUPPORT EXECUTION POLICIES YET EXECUTE ME
        // Parallel::for_each(allNodes.begin(), allNodes.end(),
        //     [&](TreeNode* node) {
        //         node->measured = node->element->measure(rootConstraints);
        //     }
        // );

        measurePhase(root, rootConstraints);
        
        Parallel::for_each(allNodes.begin(), allNodes.end(),
            [&](TreeNode* node) {
                node->atomized = node->element->atomize(rootConstraints, node->shared, *node->measured);
            }
        );
    
        // Layout pass
        // precompute margin metadata + intents
        preLayoutPhase(root, frameInfo, rootConstraints);
        // initial layout pass
        layoutPhase(root, frameInfo, rootConstraints);
        root->calculateGlobalZIndex(0);
        // postLayout: resolve global positions (serial, top-down) + reconcile atoms
        postLayoutPhase(root, frameInfo, rootConstraints, {0.0f, 0.0f}, {0.0f, 0.0f});

        Parallel::for_each(allNodes.begin(), allNodes.end(),
            [&](TreeNode* node) {
                node->placed = node->element->place(rootConstraints, node->shared, *node->measured,
                                                    *node->atomized, *node->layout);
            }
        );

        Parallel::for_each(allNodes.begin(), allNodes.end(),
            [&](TreeNode* node) {
                node->finalized = node->element->finalize(rootConstraints, node->shared, *node->measured,
                                                        *node->atomized, *node->layout, *node->placed);
            }
        );
    }

    void RenderTree::render(MTL::RenderCommandEncoder* encoder) {
        auto root = getRoot();
        auto allNodes = collectAllNodes(root);

        std::sort(allNodes.begin(), allNodes.end(), [](TreeNode* a, TreeNode* b) {
            if (a->globalZIndex != b->globalZIndex) {
                return a->globalZIndex < b->globalZIndex;
            }
            return a->id < b->id;
        });
        
        // serially encoded; encoders are not thread safe
        for (auto node : allNodes) { 
            auto& finalized = node->finalized;
            node->element->encode(encoder, finalized);
        }
    }

    void RenderTree::measurePhase(TreeNode* node, Constraints& constraints) {
        auto measured = node->element->measure(constraints, node->shared);
        node->measured = measured;
        
        Constraints childConstraints {};

        childConstraints.maxWidth = measured.explicitWidth.value_or(constraints.maxWidth);;
        childConstraints.maxHeight = measured.explicitHeight.value_or(constraints.maxHeight);

        for (auto& child : node->children) {
            measurePhase(child.get(), childConstraints);
        }
    }


    // consider safer way of accessing cache?
    void RenderTree::atomizePhase(TreeNode* node, Constraints& constraints) {
        // assert(renderCache.measured.contains(node->id) && 
        //    "atomizePhase called before measurePhase");
        assert(node->measured.has_value());


        auto& measured  = *node->measured;
        auto& shared = node->shared;
        auto atomized = node->element->atomize(constraints, shared, measured);
        node->atomized = atomized;
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
            auto position = getPosition(child.get());
            auto display = getDisplay(child.get());

            if ((position == Position::Static || position == Position::Relative) && display == Display::Block) {
                if (firstInFlowCollapsableChild == nullptr) firstInFlowCollapsableChild = child.get();
                lastInFlowCollapsableChild = child.get();
            }
        }

        // check if out of flow
        auto currPosition = getPosition(node);
        bool currOutOfFlow = currPosition == Position::Absolute || currPosition == Position::Fixed;

        if (currOutOfFlow) {
            firstInFlowCollapsableChild = nullptr;
            lastInFlowCollapsableChild = nullptr;
        }

        // skip continuation if padding defined
        bool nodeBlocksTopChain = getPaddingTop(node).has_value();
        bool nodeBlocksBottomChain = getPaddingBottom(node).has_value();

        if (nodeBlocksTopChain) {
            firstInFlowCollapsableChild = nullptr;
        }

        if (nodeBlocksBottomChain) {
            lastInFlowCollapsableChild = nullptr;
        }

        Size marginTop = getMarginTop(node);
        Size marginBottom = getMarginBottom(node);

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

    void precomputeMargins(TreeNode* node, Constraints& constraints, std::unordered_map<ChainID, CollapsedChain>& collapsedChainMap) {
        // Build replacedAttributes from collapse chain (same logic as layoutPhase)
        constraints.replacedAttributes = {};

        if (node->preLayout->marginMetadata.topChainId.has_value()) {
            auto topChain = collapsedChainMap[node->preLayout->marginMetadata.topChainId.value()];
            if (topChain.depth > 1) {
                if (node == topChain.root) {
                    constraints.replacedAttributes.marginTop = topChain.intent;
                } else {
                    constraints.replacedAttributes.marginTop = Size{};
                }
            }
        }

        if (node->preLayout->marginMetadata.bottomChainId.has_value()) {
            auto bottomChain = collapsedChainMap[node->preLayout->marginMetadata.bottomChainId.value()];
            if (bottomChain.depth > 1) {
                if (node == bottomChain.root) {
                    constraints.replacedAttributes.marginBottom = bottomChain.intent;
                } else {
                    constraints.replacedAttributes.marginBottom = Size{};
                }
            }
        }

        auto position = getPosition(node);
        auto display = getDisplay(node);

        Size marginTop = getMarginTop(node);
        Size marginRight = getMarginRight(node);
        Size marginBottom = getMarginBottom(node);
        Size marginLeft = getMarginLeft(node);

        ResolvedMargins margins{};

        if (position == Position::Static && display == Display::Block) {
            // Block normal flow: use resolveAutoMargins
            // Compute content dimensions from atoms
            float contentWidth = 0;
            float contentHeight = 0;
            for (auto& atom : node->atomized->atoms) {
                contentWidth += atom.width;
                contentHeight = std::max(contentHeight, atom.height);
            }

            float width = node->shared.width.has_value()
                ? node->shared.width->resolveOr(0.0f)
                : 0.0f;
            if (width > 0) {
                contentWidth = width;
            } else {
                contentWidth = constraints.maxWidth;
            }

            LayoutInput li{
                .position = position,
                .display = display,
                .width = width,
                .height = 0,
                .marginTop = marginTop,
                .marginRight = marginRight,
                .marginBottom = marginBottom,
                .marginLeft = marginLeft,
            };

            margins = LayoutEngine::resolveAutoMargins(li, constraints.replacedAttributes, constraints.maxWidth, contentWidth);
        } else {
            margins = {
                .top = marginTop.resolveOr(0.0f, 0.0f),
                .right = marginRight.resolveOr(0.0f, 0.0f),
                .bottom = marginBottom.resolveOr(0.0f, 0.0f),
                .left = marginLeft.resolveOr(0.0f, 0.0f),
            };
        }

        node->preLayout->resolvedMargins = margins;

        auto& measured = *node->measured;
        Constraints childConstraints{};
        childConstraints.maxWidth = measured.explicitWidth.value_or(constraints.maxWidth);
        childConstraints.maxHeight = measured.explicitHeight.value_or(constraints.maxHeight);
        childConstraints.frameInfo = constraints.frameInfo;

        for (auto& child : node->children) {
            precomputeMargins(child.get(), childConstraints, collapsedChainMap);
        }
    }

    void RenderTree::preLayoutPhase(TreeNode* node, const FrameInfo& frameInfo, Constraints& constraints) {
        collapsedChainMap.clear();
        nextChainId = 0;

        buildCollapsedChains(node, collapsedChainMap, nextChainId, nullptr, nullptr);

        precomputeMargins(node, constraints, collapsedChainMap);
    }

    std::pair<std::vector<LineFragment>, std::vector<LineBox>> buildInlineBoxesForChild(TreeNode* child, float maxWidth) {
        std::vector<LineFragment> fragments;
        std::vector<LineBox> lineBoxes;
        LineBox currentLineBox{};
        size_t currentLineBoxIndex = 0;
        bool lastFragmentHasBreakOpportunity = false;

        auto textResp = getText(child);
        if (textResp.has_value()) {
            auto margins = child->preLayout->resolvedMargins;
            auto text = *textResp;
            auto& atoms = child->atomized->atoms;

            float runningWidth = margins.left;
            size_t runningAtomCount = 0;
            size_t idx = 0;

            while (idx < text.size() && idx < atoms.size()) {
                if (text[idx] != ' ') {
                    runningWidth += atoms[idx].width;
                    runningAtomCount++;
                    idx++;
                    continue;
                }

                while (idx < text.size() && text[idx] == ' ') {
                    runningWidth += atoms[idx].width;
                    runningAtomCount++;
                    idx++;
                }

                runningWidth += margins.right;

                LineFragment frag{.width = runningWidth, .atomCount = runningAtomCount};

                if (lastFragmentHasBreakOpportunity && currentLineBox.fragmentCount > 0 && currentLineBox.width + runningWidth > maxWidth) {
                    lineBoxes.push_back(currentLineBox);
                    currentLineBox = {};
                    currentLineBoxIndex++;
                }

                frag.lineBoxIndex = currentLineBoxIndex;
                frag.fragmentIndex = currentLineBox.fragmentCount;
                fragments.push_back(frag);
                currentLineBox.pushFragment(frag);
                lastFragmentHasBreakOpportunity = true;
                runningWidth = 0.0;
                runningAtomCount = 0;
            }

            if (runningAtomCount > 0) {
                runningWidth += margins.right;
                LineFragment frag{.width = runningWidth, .atomCount = runningAtomCount};

                if (lastFragmentHasBreakOpportunity && currentLineBox.fragmentCount > 0 && currentLineBox.width + runningWidth > maxWidth) {
                    lineBoxes.push_back(currentLineBox);
                    currentLineBox = {};
                    currentLineBoxIndex++;
                }

                frag.lineBoxIndex = currentLineBoxIndex;
                frag.fragmentIndex = currentLineBox.fragmentCount;
                fragments.push_back(frag);
                currentLineBox.pushFragment(frag);
            }
        }

        if (currentLineBox.fragmentCount > 0)
            lineBoxes.push_back(currentLineBox);

        return {fragments, lineBoxes};
    }

    std::tuple<std::vector<std::vector<LineFragment>>, std::vector<LineBox>> buildInlineBoxes(TreeNode* node, Constraints& childConstraints) {
        bool prevInline = false;
        bool lastFragmentHasBreakOpportunity = false;
        std::vector<LineBox> childrenLineBoxes;
        std::vector<std::vector<LineFragment>> childrenLineFragments;
        LineBox currentLineBox {};
        size_t currentLineBoxIndex = 0;


        for (uint64_t i = 0; i < node->children.size(); ++i) {
            auto& child = node->children[i];

            std::vector<LineFragment> childLineFragments;

            auto textResp = getText(child.get());

            if (textResp.has_value()) {
                auto margins = child->preLayout->resolvedMargins;
                auto marginLeft = margins.left;
                auto marginRight = margins.right;
                auto text = *textResp;

                float runningWidth = 0.0;
                size_t runningAtomCount = 0;

                auto& atoms = child->atomized->atoms;
                size_t idx = 0;

                if (i > 0 && !prevInline && currentLineBox.fragmentCount > 0) {
                    childrenLineBoxes.push_back(currentLineBox);
                    currentLineBox = {};
                    currentLineBoxIndex++;
                }

                runningWidth += marginLeft;

                while (idx < text.size() && idx < atoms.size()) {
                    char ch = text[idx];

                    if (ch != ' ') {
                        runningWidth += atoms[idx].width;
                        runningAtomCount++;
                        idx++;
                        
                        continue;
                    }else {
                        while (idx < text.size() && text[idx] == ' ') {
                            runningWidth += atoms[idx].width;
                            runningAtomCount++;
                            idx++;
                        }

                        runningWidth += marginRight;

                        LineFragment lineFragment {
                            .width = runningWidth,
                            .atomCount = runningAtomCount,
                        };

                        if (lastFragmentHasBreakOpportunity && currentLineBox.fragmentCount > 0 && currentLineBox.width + runningWidth > childConstraints.maxWidth) {
                            childrenLineBoxes.push_back(currentLineBox);
                            currentLineBox = {};
                            currentLineBoxIndex++;
                        }

                        lineFragment.lineBoxIndex = currentLineBoxIndex;
                        lineFragment.fragmentIndex = currentLineBox.fragmentCount;

                        childLineFragments.push_back(lineFragment);
                        currentLineBox.pushFragment(lineFragment);
                        lastFragmentHasBreakOpportunity = true;

                        runningWidth = 0.0;
                        runningAtomCount = 0;
                    }
                }

                if (runningAtomCount > 0) {                        
                    runningWidth += marginRight;

                    LineFragment lineFragment {
                        .width = runningWidth,
                        .atomCount = runningAtomCount,
                    };

                    if (lastFragmentHasBreakOpportunity && currentLineBox.fragmentCount > 0 && currentLineBox.width + runningWidth > childConstraints.maxWidth) {
                        childrenLineBoxes.push_back(currentLineBox);
                        currentLineBox = {};
                        currentLineBoxIndex++;
                    }

                    lineFragment.lineBoxIndex = currentLineBoxIndex;
                    lineFragment.fragmentIndex = currentLineBox.fragmentCount;

                    childLineFragments.push_back(lineFragment);
                    currentLineBox.pushFragment(lineFragment);
                    lastFragmentHasBreakOpportunity = false;

                    runningWidth = 0.0;
                    runningAtomCount = 0;
                }

                prevInline = true;
            }else {
                prevInline = false;
            }

            childrenLineFragments.push_back(childLineFragments);
        }

        if (currentLineBox.fragmentCount > 0) {
            childrenLineBoxes.push_back(currentLineBox);
        }

        return {childrenLineFragments, childrenLineBoxes};
    }


    struct AxisHelper {
        bool isRow;
        bool isReversed;

        AxisHelper(FlexDirection dir):
            isRow{dir == FlexDirection::Row || dir == FlexDirection::RowReverse},
            isReversed{dir == FlexDirection::RowReverse || dir == FlexDirection::ColReverse}
        {}

        void applyDirection(Direction dir) {
            if (dir == Direction::rtl && isRow) isReversed = !isReversed;
        }

        float mainSize(const LayoutResult& lr) const {
            return isRow ? lr.computedBox.width : lr.computedBox.height;
        }
        float crossSize(const LayoutResult& lr) const {
            return isRow ? lr.computedBox.height : lr.computedBox.width;
        }

        void setMainExplicit(Measured& m, float v) const {
            if (isRow) m.explicitWidth = v;
            else       m.explicitHeight = v;
        }
        void setCrossExplicit(Measured& m, float v) const {
            if (isRow) m.explicitHeight = v;
            else       m.explicitWidth = v;
        }
        std::optional<float> crossExplicit(const Measured& m) const {
            return isRow ? m.explicitHeight : m.explicitWidth;
        }

        void setMainPosition(Constraints& c, float v) const {
            if (isRow) c.origin.x = v;
            else       c.cursor.y = v;
        }
        void setCrossPosition(Constraints& c, float v) const {
            if (isRow) c.cursor.y = v;
            else       c.origin.x = v;
        }

        void setMainMaxSize(Constraints& c, float v) const {
            if (isRow) c.maxWidth = v;
            else       c.maxHeight = v;
        }
        void setCrossMaxSize(Constraints& c, float v) const {
            if (isRow) c.maxHeight = v;
            else       c.maxWidth = v;
        }

        float availableMain(const Measured& m, float fallback) const {
            return isRow ? m.explicitWidth.value_or(fallback) : m.explicitHeight.value_or(fallback);
        }
        float availableCross(const Measured& m, float fallback) const {
            return isRow ? m.explicitHeight.value_or(fallback) : m.explicitWidth.value_or(fallback);
        }
    };


    struct FlexLine {
        std::vector<float> childSizes;
        std::vector<float> shrinkScaled;
        std::vector<float> growthScaled;

        float totalSize{};
        float shrinkScaledTotal{};
        float growthScaledTotal{};
        float maxCrossSize{};

        void addChild(float mainSize, float crossSize, float grow, float shrink) {
            childSizes.push_back(mainSize);
            totalSize += mainSize;

            if (shrink > 0.0f) {
                shrinkScaled.push_back(mainSize * shrink);
                shrinkScaledTotal += mainSize * shrink;
            } else {
                shrinkScaled.push_back(0.0f);
            }

            if (grow > 0.0f) {
                growthScaled.push_back(grow);
                growthScaledTotal += grow;
            } else {
                growthScaled.push_back(0.0f);
            }

            maxCrossSize = std::max(crossSize, maxCrossSize);
        }

        size_t count() const { return childSizes.size(); }

        struct ResolveResult {
            std::vector<float> sizes;
            float totalAfter{};
            float remainingSpace{};
        };

        ResolveResult resolve(float availableMain) const {
            ResolveResult result;
            float space = availableMain - totalSize;

            for (size_t i = 0; i < childSizes.size(); ++i) {
                if (space > 0 && growthScaled[i] > 0) {
                    result.sizes.push_back(childSizes[i] + (growthScaled[i] / growthScaledTotal) * space);
                } else if (space < 0 && shrinkScaled[i] > 0) {
                    result.sizes.push_back(childSizes[i] + (shrinkScaled[i] / shrinkScaledTotal) * space);
                } else {
                    result.sizes.push_back(childSizes[i]);
                }
                result.totalAfter += result.sizes.back();
            }
            result.remainingSpace = availableMain - result.totalAfter;
            return result;
        }
    };

    struct Alignment {
        float initialOffset{};
        float spaceBetween{};
    };

    enum class DistributeMode {
        FlexStart, FlexEnd, Center, SpaceBetween, SpaceAround, SpaceEvenly
    };

    Alignment distributeSpace(float remainingSpace, size_t itemCount, DistributeMode mode) {
        Alignment a;
        switch (mode) {
            case DistributeMode::FlexStart: break;
            case DistributeMode::FlexEnd:
                a.initialOffset = remainingSpace; break;
            case DistributeMode::Center:
                a.initialOffset = remainingSpace / 2.0f; break;
            case DistributeMode::SpaceBetween:
                if (itemCount > 1) a.spaceBetween = remainingSpace / (itemCount - 1); break;
            case DistributeMode::SpaceAround: {
                float gap = remainingSpace / itemCount;
                a.initialOffset = gap / 2.0f;
                a.spaceBetween = gap;
                break;
            }
            case DistributeMode::SpaceEvenly: {
                float gap = remainingSpace / (itemCount + 1);
                a.initialOffset = gap;
                a.spaceBetween = gap;
                break;
            }
        }
        return a;
    }

    DistributeMode toDistributeMode(JustifyContent jc) {
        switch (jc) {
            case JustifyContent::FlexStart:    return DistributeMode::FlexStart;
            case JustifyContent::FlexEnd:      return DistributeMode::FlexEnd;
            case JustifyContent::Center:       return DistributeMode::Center;
            case JustifyContent::SpaceBetween: return DistributeMode::SpaceBetween;
            case JustifyContent::SpaceAround:  return DistributeMode::SpaceAround;
            case JustifyContent::SpaceEvenly:  return DistributeMode::SpaceEvenly;
        }
    }

    DistributeMode toDistributeMode(AlignContent ac) {
        switch (ac) {
            case AlignContent::Stretch:      return DistributeMode::FlexStart;
            case AlignContent::FlexStart:    return DistributeMode::FlexStart;
            case AlignContent::FlexEnd:      return DistributeMode::FlexEnd;
            case AlignContent::Center:       return DistributeMode::Center;
            case AlignContent::SpaceBetween: return DistributeMode::SpaceBetween;
            case AlignContent::SpaceAround:  return DistributeMode::SpaceAround;
            case AlignContent::SpaceEvenly:  return DistributeMode::SpaceEvenly;
        }
    }


    struct FlexLayout {
        AxisHelper axis;
        JustifyContent justifyContent;
        AlignItems alignItems;
        AlignContent alignContent;
        FlexWrap flexWrap;

        std::vector<FlexLine> lines;
        FlexLine currentLine;
        std::vector<AlignSelf> childAlignSelfs;
        std::vector<float> childCrossSizes;
        float availableMain{};

        struct ChildPlacement {
            float mainOffset;
            float crossOffset;
            float mainSize;
            std::optional<float> crossSizeOverride; // for stretch
            bool needsCrossShrinkToFit{false};
        };

        FlexLayout(FlexDirection dir, JustifyContent jc, AlignItems ai,
                   AlignContent ac, FlexWrap wrap):
            axis{dir}, justifyContent{jc}, alignItems{ai},
            alignContent{ac}, flexWrap{wrap}
        {}

        void addChild(const LayoutResult& layout, float grow, float shrink,
                      AlignSelf selfAlign, float avMain) {
            float childMain = axis.mainSize(layout);
            float childCross = axis.crossSize(layout);
            availableMain = avMain;

            if (flexWrap != FlexWrap::NoWrap && currentLine.count() > 0) {
                float totalWithGap = currentLine.totalSize;
                if (totalWithGap + childMain > avMain) {
                    lines.push_back(std::move(currentLine));
                    currentLine = FlexLine{};
                }
            }

            currentLine.addChild(childMain, childCross, grow, shrink);
            childAlignSelfs.push_back(selfAlign);
            childCrossSizes.push_back(childCross);
        }

        struct ResolveResult {
            std::vector<std::vector<float>> lineSizes; 
            std::vector<float> lineTotalsAfter;       
            float overallTotalAfter{};
        };

        ResolveResult resolveSizes(float avMain) {
            if (currentLine.count() > 0) {
                lines.push_back(std::move(currentLine));
                currentLine = FlexLine{};
            }

            ResolveResult result;
            for (auto& line : lines) {
                auto lr = line.resolve(avMain);
                result.overallTotalAfter += lr.totalAfter;
                result.lineTotalsAfter.push_back(lr.totalAfter);
                result.lineSizes.push_back(std::move(lr.sizes));
            }
            return result;
        }

        std::vector<ChildPlacement> computePlacements(
            const ResolveResult& resolved,
            float availableCross, float gap
        ) {
            size_t lineCount = lines.size();

            std::vector<float> lineCrossSizes(lineCount);
            std::vector<float> lineCrossOffsets(lineCount);

            if (lineCount == 1) {
                // Single line: align-content has no effect.
                // The full cross space goes to this line; align-items handles positioning.
                std::println("availableCross: {}", availableCross);
                lineCrossSizes[0] = availableCross;
                lineCrossOffsets[0] = 0.0f;
            } else {
                // Multi-line: compute natural cross sizes, then distribute via align-content
                float totalNaturalCross = 0;
                for (size_t li = 0; li < lineCount; ++li) {
                    lineCrossSizes[li] = lines[li].maxCrossSize;
                    totalNaturalCross += lines[li].maxCrossSize;
                }

                float remainingCross = availableCross - totalNaturalCross;

                if (alignContent == AlignContent::Stretch && remainingCross > 0) {
                    float extra = remainingCross / lineCount;
                    for (size_t li = 0; li < lineCount; ++li) {
                        lineCrossSizes[li] += extra;
                    }
                    float crossAccum = 0;
                    for (size_t li = 0; li < lineCount; ++li) {
                        lineCrossOffsets[li] = crossAccum;
                        crossAccum += lineCrossSizes[li];
                    }
                } else {
                    auto crossAlign = distributeSpace(
                        remainingCross, lineCount, toDistributeMode(alignContent));
                    float crossAccum = crossAlign.initialOffset;
                    for (size_t li = 0; li < lineCount; ++li) {
                        lineCrossOffsets[li] = crossAccum;
                        crossAccum += lineCrossSizes[li] + crossAlign.spaceBetween;
                    }
                }

                // Wrap-reverse: mirror line cross offsets
                if (flexWrap == FlexWrap::WrapReverse) {
                    for (size_t li = 0; li < lineCount; ++li) {
                        lineCrossOffsets[li] = availableCross - lineCrossOffsets[li] - lineCrossSizes[li];
                    }
                }
            }

            // Build per-child placements
            std::vector<ChildPlacement> placements;
            size_t childIdx = 0;

            for (size_t li = 0; li < lineCount; ++li) {
                auto& line = lines[li];
                float lineRemainingMain = availableMain - resolved.lineTotalsAfter[li];
                auto mainAlign = distributeSpace(lineRemainingMain, line.count(), toDistributeMode(justifyContent));

                float accumulated = mainAlign.initialOffset;
                float lineCross = lineCrossSizes[li];
                float lineCrossBase = lineCrossOffsets[li];

                for (size_t ci = 0; ci < line.count(); ++ci) {
                    ChildPlacement p;
                    p.mainOffset = accumulated;
                    p.mainSize = resolved.lineSizes[li][ci];
                    
                    AlignSelf selfAlign = childAlignSelfs[childIdx];
                    AlignItems effectiveAlign = alignItems;
                    if (selfAlign != AlignSelf::Auto) {
                        switch (selfAlign) {
                            case AlignSelf::Stretch:   effectiveAlign = AlignItems::Stretch; break;
                            case AlignSelf::FlexStart: effectiveAlign = AlignItems::FlexStart; break;
                            case AlignSelf::FlexEnd:   effectiveAlign = AlignItems::FlexEnd; break;
                            case AlignSelf::Center:    effectiveAlign = AlignItems::Center; break;
                            default: break;
                        }
                    }

                    float childCross = childCrossSizes[childIdx];
                    switch (effectiveAlign) {
                        case AlignItems::Stretch:
                            p.crossOffset = lineCrossBase;
                            p.crossSizeOverride = lineCross;
                            break;
                        case AlignItems::FlexStart:
                            p.crossOffset = lineCrossBase;
                            p.needsCrossShrinkToFit = !axis.isRow;
                            break;
                        case AlignItems::Center:
                            std::println("lineCross: {}", lineCross);
                            p.crossOffset = lineCrossBase + (lineCross - childCross) / 2.0f;
                            p.needsCrossShrinkToFit = !axis.isRow;
                            break;
                        case AlignItems::FlexEnd:
                            p.crossOffset = lineCrossBase + lineCross - childCross;
                            p.needsCrossShrinkToFit = !axis.isRow;
                            break;
                    }

                    if (axis.isReversed) {
                        p.mainOffset = availableMain - accumulated - p.mainSize;
                    }

                    accumulated += resolved.lineSizes[li][ci] + mainAlign.spaceBetween + gap;
                    placements.push_back(p);
                    childIdx++;
                }
            }

            return placements;
        }
    };

    void RenderTree::layoutPhase(TreeNode* node, const FrameInfo& frameInfo, Constraints& constraints) {
        assert(node->measured.has_value());
        assert(node->atomized.has_value());
        assert(node->preLayout.has_value());

        auto& measured = *node->measured;
        auto& atomized = *node->atomized;
        auto& prelayout = *node->preLayout;

        constraints.resolvedMargins = prelayout.resolvedMargins;
        auto layout = node->element->layout(constraints, node->shared, measured, atomized);

        auto childConstraints = layout.childConstraints;
        auto position = getPosition(node);
        if (position != Position::Static) {
            childConstraints.absoluteContainingBlock = {
                .origin = {0.0f, 0.0f},
                .width = layout.computedBox.width,
                .height = layout.computedBox.height
            };
        } else {
            childConstraints.absoluteContainingBlock = constraints.absoluteContainingBlock;
        }

        float minY = layout.childConstraints.origin.y;
        float maxY = layout.childConstraints.origin.y;
        float minX = layout.childConstraints.origin.x;
        float maxX = layout.childConstraints.origin.x;

        auto display = getDisplay(node);

        if (display == Display::Flex) {

            auto flexDirection = getFlexDirection(node);
            auto justifyContent = getJustifyContent(node);
            auto alignItems = getAlignItems(node);
            auto alignContentVal = getAlignContent(node);
            auto flexWrap = getFlexWrap(node);

            FlexLayout flex{flexDirection, justifyContent, alignItems, alignContentVal, flexWrap};
            flex.axis.applyDirection(constraints.inheritedProperties.direction);

            bool needsCrossShrink = flex.axis.isRow || alignItems != AlignItems::Stretch;
            childConstraints.shrinkToFit = needsCrossShrink;

            // Compute available main for line-breaking decisions
            float avMain = flex.axis.availableMain(measured, constraints.maxWidth);
            float childMaxWidth = measured.explicitWidth.value_or(constraints.maxWidth);

            // Pass 1: intrinsic sizes
            float minIntrinsicX = minX;
            float maxIntrinsicX = maxX;
            float minIntrinsicY = minY;
            float maxIntrinsicY = maxY;

            for (uint64_t i = 0; i < node->children.size(); ++i) {
                auto childAsPtr = node->children[i].get();

                auto&& [frags, boxes] = buildInlineBoxesForChild(childAsPtr, childMaxWidth);
                childConstraints.lineFragments = frags;
                childConstraints.lineBoxes = boxes;
                childConstraints.maxWidth = childMaxWidth;
                childConstraints.inheritedProperties = constraints.inheritedProperties;

                layoutPhase(childAsPtr, frameInfo, childConstraints);
                auto& childLayout = *childAsPtr->layout;
                
                bool sizeIndefinite = !measured.explicitWidth.has_value() &&
                    (!childAsPtr->measured->explicitWidth.has_value() || childAsPtr->shared.width->unit == Unit::Percent);

                if (!sizeIndefinite) {
                    maxIntrinsicX = std::max(maxIntrinsicX, childLayout.computedBox.x + childLayout.computedBox.width);
                    maxIntrinsicY = std::max(maxIntrinsicY, childLayout.computedBox.y + childLayout.consumedHeight);
                }
            }


            // Pass 2: measure children and collect flex info
            std::vector<size_t> inFlowIndices;
            for (uint64_t i = 0; i < node->children.size(); ++i) {
                auto childAsPtr = node->children[i].get();

                auto&& [frags, boxes] = buildInlineBoxesForChild(childAsPtr, childMaxWidth);
                childConstraints.lineFragments = frags;
                childConstraints.lineBoxes = boxes;
                childConstraints.maxWidth = childMaxWidth;
                childConstraints.inheritedProperties = constraints.inheritedProperties;

                bool sizeIndefinite = !measured.explicitWidth.has_value() &&
                    (!childAsPtr->measured->explicitWidth.has_value() || childAsPtr->shared.width->unit == Unit::Percent);

                if (sizeIndefinite) {
                    childAsPtr->measured->explicitWidth = maxIntrinsicX - minIntrinsicX;
                }
                
                layoutPhase(childAsPtr, frameInfo, childConstraints);
                auto& childLayout = *childAsPtr->layout;

                if (childLayout.outOfFlow) continue;

                float resolvedGrow = getFlexGrow(childAsPtr).resolveOr(0.0, 0.0);
                float resolvedShrink = getFlexShrink(childAsPtr).resolveOr(0.0, 1.0);
                auto selfAlign = getAlignSelf(childAsPtr);
                flex.addChild(childLayout, resolvedGrow, resolvedShrink, selfAlign, avMain);
                inFlowIndices.push_back(i);
            }

            float totalSizeFallback = 0;
            for (auto& line : flex.lines) totalSizeFallback += line.totalSize;
            totalSizeFallback += flex.currentLine.totalSize;
            auto explicitMain = flex.axis.isRow ? measured.explicitWidth : measured.explicitHeight;
            float availableMain = explicitMain.has_value()
                ? (flex.axis.isRow ? layout.childConstraints.maxWidth : layout.childConstraints.maxHeight)
                : totalSizeFallback;
            flex.availableMain = availableMain;

            auto resolved = flex.resolveSizes(availableMain);

            float gapBasis = flex.axis.isRow
                ? measured.explicitWidth.value_or(constraints.maxWidth)
                : measured.explicitHeight.value_or(resolved.overallTotalAfter);
            float resolvedGap = getFlexGap(node).resolveOr(gapBasis);

            float naturalCross = 0;

            // setting natural cross as the fallback
            for (auto& line : flex.lines) naturalCross += line.maxCrossSize;
            auto explicitCross = flex.axis.isRow ? measured.explicitHeight : measured.explicitWidth;
            float availableCross = explicitCross.has_value()
                ? (flex.axis.isRow ? layout.childConstraints.maxHeight : layout.childConstraints.maxWidth)
                : naturalCross;

            auto placements = flex.computePlacements(resolved, availableCross, resolvedGap);
            childConstraints.shrinkToFit = false;

            // Pass 3: apply placements and re-layout

            for (size_t pi = 0; pi < inFlowIndices.size(); ++pi) {
                size_t i = inFlowIndices[pi];
                auto childAsPtr = node->children[i].get();
                auto& p = placements[pi];

                auto&& [frags, boxes] = buildInlineBoxesForChild(childAsPtr, childMaxWidth);
                childConstraints.lineFragments = frags;
                childConstraints.lineBoxes = boxes;
                childConstraints.inheritedProperties = constraints.inheritedProperties;

                // Main axis
                flex.axis.setMainPosition(childConstraints, p.mainOffset);
                flex.axis.setMainMaxSize(childConstraints, p.mainSize);
                flex.axis.setMainExplicit(*childAsPtr->measured, p.mainSize);

                // Cross axis: col needs maxWidth constraint; row doesn't need maxHeight
                if (!flex.axis.isRow) {
                    flex.axis.setCrossMaxSize(childConstraints, availableCross);
                }
                childConstraints.shrinkToFit = p.needsCrossShrinkToFit;

                // Cross-axis: apply placement offset and stretch override
                flex.axis.setCrossPosition(childConstraints, p.crossOffset);
                if (p.crossSizeOverride.has_value()) {
                    if (!flex.axis.crossExplicit(*childAsPtr->measured).has_value()) {
                        flex.axis.setCrossExplicit(*childAsPtr->measured, *p.crossSizeOverride);
                    }
                }

                layoutPhase(childAsPtr, frameInfo, childConstraints);
                auto& childLayout = *childAsPtr->layout;

                maxX = std::max(maxX, childLayout.computedBox.x + childLayout.computedBox.width);
                maxY = std::max(maxY, childLayout.computedBox.y + childLayout.consumedHeight);
            }
        } else {
            auto&& [childrenLineFragments, childrenLineBoxes] = buildInlineBoxes(node, childConstraints);
            childConstraints.lineBoxes = childrenLineBoxes;

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
        }

        // percent/auto does not contribute to parent intrinsic size. But we don't use intrinsic sizes to compute this. huh
        
        // resize width/height of underspecified elements
        if (!measured.explicitWidth.has_value()) {
            if (position != Position::Static || constraints.shrinkToFit) {
                layout.computedBox.width = maxX - minX + layout.resolvedPadding.left + layout.resolvedPadding.right;
            }
        }

        if (!measured.explicitHeight.has_value()) {
            layout.computedBox.height = maxY - minY + layout.resolvedPadding.top + layout.resolvedPadding.bottom;
            layout.consumedHeight = layout.computedBox.height;
        }

        // finalize layout of node
        node->layout = layout;
    }

    void RenderTree::postLayoutPhase(TreeNode* node, const FrameInfo& frameInfo, Constraints& constraints,
                                      simd_float2 parentGlobalOrigin, simd_float2 absBlockGlobalOrigin) {
        assert(node->measured.has_value());
        assert(node->atomized.has_value());
        assert(node->layout.has_value());

        auto& layout = *node->layout;
        auto position = getPosition(node);

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

        node->atomized = node->element->postLayout(constraints, node->shared, *node->measured,
                                                    *node->atomized, layout);

        simd_float2 currContentOrigin = {
            layout.computedBox.x + layout.resolvedPadding.left,
            layout.computedBox.y + layout.resolvedPadding.top
        };

        simd_float2 childAbsBlockOrigin = absBlockGlobalOrigin;
        if (position != Position::Static) {
            childAbsBlockOrigin = currContentOrigin;
        }

        for (auto& child : node->children) {
            postLayoutPhase(child.get(), frameInfo, constraints,
                           currContentOrigin, childAbsBlockOrigin);
        }
    }

    void RenderTree::placePhase(TreeNode* node, const FrameInfo& frameInfo, Constraints& constraints) {
        assert(node->measured.has_value());
        assert(node->atomized.has_value());
        assert(node->layout.has_value());
        
        auto& measured = *node->measured;
        auto& atomized = *node->atomized;
        auto& layout = *node->layout;

        auto placed = node->element->place(constraints, node->shared, measured, atomized, layout);
        node->placed = placed;
    }

    void RenderTree::finalizePhase(TreeNode* node, Constraints& constraints) {
        // I thought runtime checks for private methods were stupid, but I also am not superhuman and LLMs can hallucinate,
        // so debug asserts never hurt
        assert(node->measured.has_value());
        assert(node->atomized.has_value());
        assert(node->finalized.has_value());
        
            
        auto& measured =  *node->measured;
        auto& atomized = *node->atomized;
        auto& layout = *node->layout;
        auto& placed = *node->placed;
        auto finalized = node->element->finalize(constraints, node->shared, measured, atomized, layout, placed);
        node->finalized = finalized;
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
