#include "render_tree.hpp"

namespace NewArch {
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
        float parentMaxWidth  = childConstraints.maxWidth;
        float parentMaxHeight = childConstraints.maxHeight;
        float originX         = childConstraints.origin.x;
        float originY         = childConstraints.origin.y;

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

        float minY = originY;
        float maxY = originY;
        float minX = originX;
        float maxX = originX;

        auto display = getDisplay(node);

        if (display == Display::Flex) {
            auto flexDirection = getFlexDirection(node);
            auto justifyContent = getJustifyContent(node);
            auto alignItems = getAlignItems(node);
            auto alignContentVal = getAlignContent(node);
            auto flexWrap = getFlexWrap(node);

            FlexLayout flexContext {flexDirection, justifyContent, alignItems, alignContentVal, flexWrap};
            flexContext.axis.applyDirection(constraints.inheritedProperties.direction);

            FlexResolver fr {
                *this, node, constraints, childConstraints, flexContext, frameInfo, parentMaxWidth, parentMaxHeight, minX, minY, maxX, maxY
            };

            fr.phaseAB();
            fr.phaseC();
            auto bounds = fr.phaseD();

            maxX = bounds.maxX;
            maxY = bounds.maxY;
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

                    auto&& [frags, boxes] = buildIsolatedInlineBoxes(childAsPtr, childMaxWidth);
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

                    auto&& [frags, boxes] = buildIsolatedInlineBoxes(childAsPtr, childMaxWidth);
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
                    auto&& [frags, boxes] = buildIsolatedInlineBoxes(childAsPtr, childMaxWidth);
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

            float contentWidth = parentMaxWidth;
            float contentHeight = parentMaxHeight;
            auto colTracks = resolveTracks(templateCols, contentWidth, colGap, true);
            auto rowTracks = resolveTracks(rowDefs, contentHeight, rowGap, false);

            // Phase D: apply placements and re-layout
            for (auto& item : items) {
                auto childAsPtr = node->children[item.childIdx].get();

                float cellX = colTracks[item.c0].offset;
                float cellY = rowTracks[item.r0].offset;
                float cellW = colTracks[item.c1 - 1].offset + colTracks[item.c1 - 1].size - cellX;
                float cellH = rowTracks[item.r1 - 1].offset + rowTracks[item.r1 - 1].size - cellY;

                auto&& [frags, boxes] = buildIsolatedInlineBoxes(childAsPtr, cellW);
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