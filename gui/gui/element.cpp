#include "element.hpp"
#include "fragment_types.hpp"
#include "sizing.hpp"
#include "new_arch.hpp"
#include "printers.hpp"
#include <algorithm>
#include <any>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <print>
#include <ranges>
#include <simd/vector_types.h>
#include <string>
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
    const std::vector<Size>& getGridTemplateColumns(TreeNode* node) { return node->shared.gridTemplateColumns; }
    const std::vector<Size>& getGridTemplateRows(TreeNode* node) { return node->shared.gridTemplateRows; }
    Size getGridColumnGap(TreeNode* node) { return node->shared.gridColumnGap; }
    Size getGridRowGap(TreeNode* node) { return node->shared.gridRowGap; }
    GridPlacement getGridPlacement(TreeNode* node) { return node->shared.gridPlacement; }

    // Element-specific requests still use the request system
    std::optional<std::u32string> getText(TreeNode* node) {
        std::any request{DescriptorPayload{GetField{.name = "text"}}};
        auto resp = node->element->request(RequestTarget::Descriptor, request);
        if (resp.has_value()) {
            return std::any_cast<std::u32string>(resp);
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
        
        float paddingLeft = node->shared.paddingLeft.value_or(Size{}).resolveOr(constraints.maxWidth);
        float paddingTop = node->shared.paddingTop.value_or(Size{}).resolveOr(constraints.maxHeight);
        float paddingRight = node->shared.paddingRight.value_or(Size{}).resolveOr(constraints.maxWidth);
        float paddingBottom = node->shared.paddingBottom.value_or(Size{}).resolveOr(constraints.maxHeight);

        Constraints childConstraints {};

        // std::println("padidngLeft: {} paddingRight: {}", paddingLeft, paddingRight);
    
        childConstraints.maxWidth = measured.explicitWidth.value_or(constraints.maxWidth) - paddingLeft - paddingRight;
        childConstraints.maxHeight = measured.explicitHeight.value_or(constraints.maxHeight) - paddingTop - paddingBottom;
        
        // std::println("maxWidth: {}", childConstraints.maxWidth);

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

        AxisHelper() {};

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
        std::vector<float> minMainSizes;
        std::vector<float> shrinkScaled;
        std::vector<float> growthScaled;

        float totalSize{};
        float shrinkScaledTotal{};
        float growthScaledTotal{};
        float maxCrossSize{};

        void addChild(float mainSize, float crossSize, float grow, float shrink, float minMainSize = 0.0f) {
            childSizes.push_back(mainSize);
            minMainSizes.push_back(minMainSize);
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
                    float shrunk = childSizes[i] + (shrinkScaled[i] / shrinkScaledTotal) * space;
                    result.sizes.push_back(std::max(shrunk, minMainSizes[i]));
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
            std::optional<float> crossSizeOverride; 
            bool needsCrossShrinkToFit{false};
        };

        FlexLayout() {}

        FlexLayout(FlexDirection dir, JustifyContent jc, AlignItems ai,
                   AlignContent ac, FlexWrap wrap):
            axis{dir}, justifyContent{jc}, alignItems{ai},
            alignContent{ac}, flexWrap{wrap}
        {}

        void addChild(const LayoutResult& layout, float grow, float shrink,
                      AlignSelf selfAlign, float avMain, float minMainSize = 0.0f) {
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

            currentLine.addChild(childMain, childCross, grow, shrink, minMainSize);
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
                lineCrossSizes[0] = availableCross;
                lineCrossOffsets[0] = 0.0f;
            } else {
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
                .availableWidth = constraints.maxWidth,
                .availableHeight = constraints.maxHeight
            };

            auto newSize = resolveSize(sizeCtx);
            
            node->measured->explicitWidth = newSize.width;
            node->measured->explicitHeight = newSize.height;
        }


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

            bool needsCrossShrink = flex.axis.isRow || alignItems != AlignItems::Stretch || !measured.explicitWidth.has_value();
            childConstraints.shrinkToFit = needsCrossShrink;

            float avMain = flex.axis.availableMain(measured, constraints.maxWidth);
            float childMaxWidth = measured.explicitWidth.value_or(constraints.maxWidth);

            auto isXIndefinite = [&](TreeNode* child) {
                return !measured.explicitWidth.has_value() &&
                    (child->shared.width.has_value() && child->shared.width->unit == Unit::Percent);
            };

            auto isYIndefinite = [&](TreeNode* child) {
                return !measured.explicitHeight.has_value() &&
                    (child->shared.height.has_value() && child->shared.height->unit == Unit::Percent);
            };

            bool hasIndefiniteChild = false;
            for (auto& child : node->children) {
                if (isXIndefinite(child.get()) || isYIndefinite(child.get())) {
                    hasIndefiniteChild = true;
                    break;
                }
            }

            // Phase A+B: resolve intrinsic sizes for indefinite children
            if (hasIndefiniteChild) {
                float maxIntrinsicX = minX;
                float maxIntrinsicY = minY;
                
                // Phase A: lay out all children to determine intrinsic content size.
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

                    auto&& [frags, boxes] = buildInlineBoxesForChild(childAsPtr, childMaxWidth);
                    childConstraints.lineFragments = frags;
                    childConstraints.lineBoxes = boxes;
                    childConstraints.maxWidth = childMaxWidth;
                    childConstraints.inheritedProperties = constraints.inheritedProperties;
                    bool isIndef = xIndef || yIndef;
                    bool savedShrink = childConstraints.shrinkToFit;
                    if (isIndef) childConstraints.shrinkToFit = true;

                    layoutPhase(childAsPtr, frameInfo, childConstraints);
                    auto& childLayout = *childAsPtr->layout;

                    maxIntrinsicX = std::max(maxIntrinsicX, childLayout.computedBox.x + childLayout.computedBox.width);
                    maxIntrinsicY = std::max(maxIntrinsicY, childLayout.computedBox.y + childLayout.consumedHeight);

                    if (xIndef) childAsPtr->measured->explicitWidth = savedWidth;
                    if (yIndef) childAsPtr->measured->explicitHeight = savedHeight;
                    childConstraints.shrinkToFit = savedShrink;
                    childConstraints.cursor = childLayout.siblingCursor;
                }


                // Phase B: lay out indefinite children with corrected cross-axis sizes
                for (uint64_t i = 0; i < node->children.size(); ++i) {
                    auto childAsPtr = node->children[i].get();
                    bool xIndef = isXIndefinite(childAsPtr);
                    bool yIndef = isYIndefinite(childAsPtr);

                    if (!xIndef && !yIndef) continue;


                    bool setCrossWidth = xIndef && !flex.axis.isRow;
                    bool setCrossHeight = yIndef && flex.axis.isRow;

                    if (!setCrossWidth && !setCrossHeight) continue;

                    if (setCrossWidth) childAsPtr->measured->explicitWidth = maxIntrinsicX - minX;
                    if (setCrossHeight) childAsPtr->measured->explicitHeight = maxIntrinsicY - minY;

                    // relayout step really only covers edge cases with text wrapping
                    auto&& [frags, boxes] = buildInlineBoxesForChild(childAsPtr, childMaxWidth);
                    childConstraints.lineFragments = frags;
                    childConstraints.lineBoxes = boxes;
                    childConstraints.maxWidth = childMaxWidth;
                    childConstraints.inheritedProperties = constraints.inheritedProperties;

                    layoutPhase(childAsPtr, frameInfo, childConstraints);

                    auto& childLayout = *childAsPtr->layout;
                    childConstraints.cursor = childLayout.siblingCursor;
                }
            }

            // Phase C: collect flex info from all children (using cached layout results)
            std::vector<size_t> inFlowIndices;

            for (uint64_t i = 0; i < node->children.size(); ++i) {
                auto childAsPtr = node->children[i].get();

                if (!hasIndefiniteChild) {
                    auto&& [frags, boxes] = buildInlineBoxesForChild(childAsPtr, childMaxWidth);
                    childConstraints.lineFragments = frags;
                    childConstraints.lineBoxes = boxes;
                    childConstraints.maxWidth = childMaxWidth;

                    childConstraints.inheritedProperties = constraints.inheritedProperties;

                    layoutPhase(childAsPtr, frameInfo, childConstraints);
                }

                auto& childLayout = *childAsPtr->layout;
                if (childLayout.outOfFlow) continue;

                float resolvedGrow = getFlexGrow(childAsPtr).resolveOr(0.0, 0.0);
                float resolvedShrink = getFlexShrink(childAsPtr).resolveOr(0.0, 1.0);
                auto selfAlign = getAlignSelf(childAsPtr);

                bool hasExplicitMain = flex.axis.isRow
                    ? childAsPtr->shared.width.has_value()
                    : childAsPtr->shared.height.has_value();
                    
                float minMain = hasExplicitMain ? 0.0f : flex.axis.mainSize(childLayout);

                flex.addChild(childLayout, resolvedGrow, resolvedShrink, selfAlign, avMain, minMain);
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

            for (auto& line : flex.lines) naturalCross += line.maxCrossSize;
            auto explicitCross = flex.axis.isRow ? measured.explicitHeight : measured.explicitWidth;
            float availableCross = explicitCross.has_value()
                ? (flex.axis.isRow ? layout.childConstraints.maxHeight : layout.childConstraints.maxWidth)
                : naturalCross;

            auto placements = flex.computePlacements(resolved, availableCross, resolvedGap);
            childConstraints.shrinkToFit = false;

            // Phase D: apply placements and re-layout
            for (size_t pi = 0; pi < inFlowIndices.size(); ++pi) {
                size_t i = inFlowIndices[pi];
                auto childAsPtr = node->children[i].get();
                auto& p = placements[pi];

                auto&& [frags, boxes] = buildInlineBoxesForChild(childAsPtr, childMaxWidth);
                childConstraints.lineFragments = frags;
                childConstraints.lineBoxes = boxes;
                childConstraints.inheritedProperties = constraints.inheritedProperties;

                flex.axis.setMainPosition(childConstraints, p.mainOffset);
                flex.axis.setMainMaxSize(childConstraints, p.mainSize);
                flex.axis.setMainExplicit(*childAsPtr->measured, p.mainSize);

                // Cross axis: col needs maxWidth constraint; row doesn't need maxHeight
                if (!flex.axis.isRow) {
                    flex.axis.setCrossMaxSize(childConstraints, availableCross);
                }
                childConstraints.shrinkToFit = p.needsCrossShrinkToFit;

                // Cross-axis: apply placement offset and stretch override.
                // Check the original spec (shared), not measured, because measured
                // can be corrupted by Phase B from an earlier intermediate pass.
                flex.axis.setCrossPosition(childConstraints, p.crossOffset);
                if (p.crossSizeOverride.has_value()) {
                    bool hasUserCrossSize = flex.axis.isRow
                        ? childAsPtr->shared.height.has_value()
                        : childAsPtr->shared.width.has_value();
                    if (!hasUserCrossSize) {
                        flex.axis.setCrossExplicit(*childAsPtr->measured, *p.crossSizeOverride);
                    }
                }

                layoutPhase(childAsPtr, frameInfo, childConstraints);
                auto& childLayout = *childAsPtr->layout;

                maxX = std::max(maxX, childLayout.computedBox.x + childLayout.computedBox.width);
                maxY = std::max(maxY, childLayout.computedBox.y + childLayout.consumedHeight);
            }
        } else if (display == Display::Grid) {
            auto& templateCols = getGridTemplateColumns(node);
            auto& templateRows = getGridTemplateRows(node);
            float availableWidth = measured.explicitWidth.value_or(constraints.maxWidth);
            float availableHeight = measured.explicitHeight.value_or(constraints.maxHeight);
            float colGap = getGridColumnGap(node).resolveOr(availableWidth, 0);
            float rowGap = getGridRowGap(node).resolveOr(availableHeight, 0);
            auto gridAlignItems = getAlignItems(node);

            size_t numCols = templateCols.size();
            // mutable copy of row templates (may grow with implicit rows)
            std::vector<Size> rowDefs(templateRows.begin(), templateRows.end());
            size_t numRows = rowDefs.size();

            float childMaxWidth = measured.explicitWidth.value_or(constraints.maxWidth);

            // Phase A+B: resolve intrinsic sizes for indefinite children
            auto isXIndefinite = [&](TreeNode* child) {
                return !measured.explicitWidth.has_value() &&
                    child->shared.width.has_value() && child->shared.width->unit == Unit::Percent;
            };
            auto isYIndefinite = [&](TreeNode* child) {
                return !measured.explicitHeight.has_value() &&
                    child->shared.height.has_value() && child->shared.height->unit == Unit::Percent;
            };

            bool hasIndefiniteChild = false;
            for (auto& child : node->children) {
                if (isXIndefinite(child.get()) || isYIndefinite(child.get())) {
                    hasIndefiniteChild = true;
                    break;
                }
            }

            if (hasIndefiniteChild) {
                float maxIntrinsicX = minX;
                float maxIntrinsicY = minY;

                // Phase A: lay out all children; indefinite ones get shrink-to-fit
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

                    auto&& [frags, boxes] = buildInlineBoxesForChild(childAsPtr, childMaxWidth);
                    childConstraints.lineFragments = frags;
                    childConstraints.lineBoxes = boxes;
                    childConstraints.maxWidth = childMaxWidth;
                    childConstraints.inheritedProperties = constraints.inheritedProperties;
                    bool isIndef = xIndef || yIndef;
                    bool savedShrink = childConstraints.shrinkToFit;
                    if (isIndef) childConstraints.shrinkToFit = true;

                    layoutPhase(childAsPtr, frameInfo, childConstraints);
                    auto& childLayout = *childAsPtr->layout;

                    maxIntrinsicX = std::max(maxIntrinsicX, childLayout.computedBox.x + childLayout.computedBox.width);
                    maxIntrinsicY = std::max(maxIntrinsicY, childLayout.computedBox.y + childLayout.consumedHeight);

                    if (xIndef) childAsPtr->measured->explicitWidth = savedWidth;
                    if (yIndef) childAsPtr->measured->explicitHeight = savedHeight;
                    childConstraints.shrinkToFit = savedShrink;
                }

                // Phase B: lay out indefinite children with corrected sizes.
                // Save/restore measured so modifications don't leak to subsequent passes.
                for (uint64_t i = 0; i < node->children.size(); ++i) {
                    auto childAsPtr = node->children[i].get();
                    bool xIndef = isXIndefinite(childAsPtr);
                    bool yIndef = isYIndefinite(childAsPtr);
                    if (!xIndef && !yIndef) continue;

                    if (xIndef) childAsPtr->measured->explicitWidth = maxIntrinsicX - minX;
                    if (yIndef) childAsPtr->measured->explicitHeight = maxIntrinsicY - minY;

                    auto&& [frags, boxes] = buildInlineBoxesForChild(childAsPtr, childMaxWidth);
                    childConstraints.lineFragments = frags;
                    childConstraints.lineBoxes = boxes;
                    childConstraints.maxWidth = childMaxWidth;
                    childConstraints.inheritedProperties = constraints.inheritedProperties;
                    layoutPhase(childAsPtr, frameInfo, childConstraints);
                }
            }

            // Phase C1: resolve item placements
            struct ItemPlacement { size_t childIdx; int c0, c1, r0, r1; };
            std::vector<ItemPlacement> items;
            std::vector<std::vector<bool>> occupied(numRows, std::vector<bool>(numCols, false));
            int autoCol = 0, autoRow = 0;

            for (size_t i = 0; i < node->children.size(); ++i) {
                auto childAsPtr = node->children[i].get();
                auto childPos = getPosition(childAsPtr);
                if (childPos == Position::Absolute || childPos == Position::Fixed) continue;

                auto gp = getGridPlacement(childAsPtr);
                int c0, c1, r0, r1;

                if (gp.colStart != 0) {
                    c0 = gp.colStart - 1;
                    c1 = (gp.colEnd != 0) ? gp.colEnd - 1 : c0 + 1;
                } else { c0 = -1; c1 = -1; }

                if (gp.rowStart != 0) {
                    r0 = gp.rowStart - 1;
                    r1 = (gp.rowEnd != 0) ? gp.rowEnd - 1 : r0 + 1;
                } else { r0 = -1; r1 = -1; }

                // Auto-placement for unplaced axes
                if (c0 == -1 || r0 == -1) {
                    int spanCols = (c0 != -1) ? (c1 - c0) : 1;
                    int spanRows = (r0 != -1) ? (r1 - r0) : 1;

                    while (true) {
                        if (autoCol + spanCols > (int)numCols) { autoCol = 0; autoRow++; }
                        if (autoRow + spanRows > (int)numRows) {
                            numRows = autoRow + spanRows;
                            rowDefs.resize(numRows, Size::autoSize());
                            occupied.resize(numRows, std::vector<bool>(numCols, false));
                        }
                        bool free = true;
                        for (int r = autoRow; r < autoRow + spanRows && free; ++r)
                            for (int c = autoCol; c < autoCol + spanCols && free; ++c)
                                if (occupied[r][c]) free = false;
                        if (free) break;
                        autoCol++;
                    }
                    if (c0 == -1) { c0 = autoCol; c1 = autoCol + spanCols; }
                    if (r0 == -1) { r0 = autoRow; r1 = autoRow + spanRows; }
                    autoCol = c1;
                    if (autoCol >= (int)numCols) { autoCol = 0; autoRow++; }
                }

                // Mark occupied
                for (int r = r0; r < r1 && r < (int)numRows; ++r)
                    for (int c = c0; c < c1 && c < (int)numCols; ++c)
                        occupied[r][c] = true;

                items.push_back({i, c0, c1, r0, r1});
            }

         // Phase C2: initial child layout (for auto track sizing)
            for (auto& item : items) {
                auto childAsPtr = node->children[item.childIdx].get();
                if (!childAsPtr->layout.has_value() || !hasIndefiniteChild) {
                    auto&& [frags, boxes] = buildInlineBoxesForChild(childAsPtr, childMaxWidth);
                    childConstraints.lineFragments = frags;
                    childConstraints.lineBoxes = boxes;
                    childConstraints.maxWidth = childMaxWidth;
                    childConstraints.inheritedProperties = constraints.inheritedProperties;
                    layoutPhase(childAsPtr, frameInfo, childConstraints);
                }
            }

            // Phase C3: track sizing
            struct TrackInfo { float offset; float size; };

            // make this not a lambda??? should be in its own data structure in my refactor
            auto resolveTracks = [&](const std::vector<Size>& defs, float available, float gap,
                                     bool isCol) -> std::vector<TrackInfo> {
                size_t n = defs.size();
                if (n == 0) return {};
                float totalGap = (n > 1) ? gap * (float)(n - 1) : 0;
                float usable = available - totalGap;

                std::vector<float> sizes(n, 0);
                float fixedTotal = 0;
                float frTotal = 0;

                // Pass 1: fixed tracks
                for (size_t t = 0; t < n; ++t) {
                    if (defs[t].isFr()) {
                        frTotal += defs[t].value;
                    } else if (!defs[t].isAuto()) {
                        sizes[t] = defs[t].resolveOr(available, 0);
                        fixedTotal += sizes[t];
                    }
                }

                // Pass 2: auto tracks (max child natural size among single-track items)
                float autoTotal = 0;
                for (size_t t = 0; t < n; ++t) {
                    if (!defs[t].isAuto()) continue;
                    float maxChildSize = 0;
                    for (auto& item : items) {
                        int s = isCol ? item.c0 : item.r0;
                        int e = isCol ? item.c1 : item.r1;
                        if (s == (int)t && e == s + 1) {
                            auto& cl = *node->children[item.childIdx]->layout;
                            float sz = isCol ? cl.computedBox.width : cl.computedBox.height;
                            maxChildSize = std::max(maxChildSize, sz);
                        }
                    }
                    sizes[t] = maxChildSize;
                    autoTotal += maxChildSize;
                }

                // Pass 3: fr tracks
                float remaining = std::max(0.0f, usable - fixedTotal - autoTotal);
                if (frTotal > 0) {
                    for (size_t t = 0; t < n; ++t) {
                        if (defs[t].isFr()) {
                            sizes[t] = (defs[t].value / frTotal) * remaining;
                        }
                    }
                }

                // Compute offsets
                std::vector<TrackInfo> tracks(n);
                float offset = 0;
                for (size_t t = 0; t < n; ++t) {
                    tracks[t] = {offset, sizes[t]};
                    offset += sizes[t] + gap;
                }
                return tracks;
            };

            float contentWidth = layout.childConstraints.maxWidth;
            float contentHeight = layout.childConstraints.maxHeight;
            auto colTracks = resolveTracks(templateCols, contentWidth, colGap, true);
            auto rowTracks = resolveTracks(rowDefs, contentHeight, rowGap, false);

            // Phase D: apply placements and re-layout
            for (auto& item : items) {
                auto childAsPtr = node->children[item.childIdx].get();

                float cellX = colTracks[item.c0].offset;
                float cellY = rowTracks[item.r0].offset;
                float cellW = colTracks[item.c1 - 1].offset + colTracks[item.c1 - 1].size - cellX;
                float cellH = rowTracks[item.r1 - 1].offset + rowTracks[item.r1 - 1].size - cellY;

                auto&& [frags, boxes] = buildInlineBoxesForChild(childAsPtr, cellW);
                childConstraints.lineFragments = frags;
                childConstraints.lineBoxes = boxes;
                childConstraints.maxWidth = cellW;
                childConstraints.maxHeight = cellH;
                childConstraints.origin.x = cellX;
                childConstraints.cursor.y = cellY;
                childConstraints.inheritedProperties = constraints.inheritedProperties;
                childConstraints.shrinkToFit = false;

                // Resolve alignment
                AlignItems effectiveAlign = gridAlignItems;
                auto selfAlign = getAlignSelf(childAsPtr);
                if (selfAlign != AlignSelf::Auto) {
                    switch (selfAlign) {
                        case AlignSelf::Stretch:   effectiveAlign = AlignItems::Stretch; break;
                        case AlignSelf::FlexStart: effectiveAlign = AlignItems::FlexStart; break;
                        case AlignSelf::FlexEnd:   effectiveAlign = AlignItems::FlexEnd; break;
                        case AlignSelf::Center:    effectiveAlign = AlignItems::Center; break;
                        default: break;
                    }
                }

                // Stretch: override measured size if child has no explicit size
                if (effectiveAlign == AlignItems::Stretch) {
                    if (!childAsPtr->measured->explicitWidth.has_value())
                        childAsPtr->measured->explicitWidth = cellW;
                    if (!childAsPtr->measured->explicitHeight.has_value())
                        childAsPtr->measured->explicitHeight = cellH;
                } else {
                    childConstraints.shrinkToFit = true;
                }

                layoutPhase(childAsPtr, frameInfo, childConstraints);
                auto& childLayout = *childAsPtr->layout;

                // Non-stretch alignment: offset within cell
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
