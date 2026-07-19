#include "inspector.hpp"
#include "context_manager.hpp"
#include "events.hpp"
#include "instrumentation.hpp"
#include "new_arch.hpp"
#include "sizing.hpp"
#include <algorithm>
#include <functional>
#include <print>
#include <string>
#include <unordered_map>
#include <vector>
#include "fonts.hpp"

namespace Inspector {
    namespace {
        constexpr int KEY_CODE_UP = 126;
        constexpr int KEY_CODE_DOWN = 125;

        std::string displayName(style::Display display) {
            switch (display) {
                case style::Display::Block: return "Block";
                case style::Display::Inline: return "Inline";
                case style::Display::Flex: return "Flex";
                case style::Display::Grid: return "Grid";
            }
        }

        std::string positionName(style::Position position) {
            switch (position) {
                case style::Position::Absolute: return "Absolute";
                case style::Position::Fixed: return "Fixed";
                case style::Position::Static: return "Static";
                case style::Position::Relative: return "Relative";
            }
        }

        std::string overflowName(style::Overflow overflow) {
            switch (overflow) {
                case style::Overflow::Visible: return "Visible";
                case style::Overflow::Hidden: return "Hidden";
                case style::Overflow::Scroll: return "Scroll";
            }
        }

        std::string_view phaseName(instrumentation::Phase phase) {
            using instrumentation::Phase;
            switch (phase) {
                case Phase::Update: return "update";
                case Phase::Measure: return "measure";
                case Phase::Atomize: return "atomize";
                case Phase::PreLayout: return "pre-layout";
                case Phase::Layout: return "layout";
                case Phase::PostLayout: return "post-layout";
                case Phase::Place: return "place";
                case Phase::Finalize: return "finalize";
                case Phase::Render: return "render";
                case Phase::Count: return "unknown";
            }
        }

        std::string_view recomputeReasonName(instrumentation::RecomputeReason reason) {
            using instrumentation::RecomputeReason;
            switch (reason) {
                case RecomputeReason::None: return "cached";
                case RecomputeReason::Dirty: return "dirty";
                case RecomputeReason::MissingConstraintsKey: return "no key";
                case RecomputeReason::ConstraintsChanged: return "constraints";
                case RecomputeReason::AncestorRecomputed: return "ancestor";
            }
        }

        double milliseconds(std::chrono::nanoseconds duration) {
            return std::chrono::duration<double, std::milli>(duration).count();
        }

        bool hasPropagation(
            instrumentation::DirtyPropagation value,
            instrumentation::DirtyPropagation flag
        ) {
            return (std::to_underlying(value) & std::to_underlying(flag)) != 0;
        }

        bool isAncestorOf(const tree::TreeNode* ancestor, const tree::TreeNode* node) {
            for (auto* current = node; current; current = current->parent) {
                if (current == ancestor) return true;
            }
            return false;
        }

        std::string_view fileName(std::string_view path) {
            auto separator = path.find_last_of("/\\");
            return separator == std::string_view::npos ? path : path.substr(separator + 1);
        }
    }

    Inspector::Inspector():
        mouseXText{text(mouseXLabel()).fontSize(style::Size::pt(12.0)).font(Menlo).color(simd_float4{1.0,1.0,1.0,1.0})},
        mouseYText{text(mouseYLabel()).fontSize(style::Size::pt(12.0)).font(Menlo).color(simd_float4{1.0,1.0,1.0,1.0})},
        nodeIDText{text(nodeIdLabel()).fontSize(style::Size::pt(12.0)).font(Menlo).color(simd_float4{1.0,1.0,1.0,1.0})},
        nodeTypeText{text(nodeTypeLabel()).fontSize(style::Size::pt(12.0)).font(Menlo).color(simd_float4{1.0,1.0,1.0,1.0})},
        hitStackText{text(hitStackLabel()).fontSize(style::Size::pt(12.0)).font(Menlo).color(simd_float4{1.0,1.0,1.0,1.0})},
        nodeStyleText{text(nodeStyleLabel()).fontSize(style::Size::pt(12.0)).font(Menlo).color(simd_float4{1.0,1.0,1.0,1.0})},
        nodePositionText{text(nodePositionLabel()).fontSize(style::Size::pt(12.0)).font(Menlo).color(simd_float4{1.0,1.0,1.0,1.0})},
        nodeWText{text(nodeWidthLabel()).fontSize(style::Size::pt(12.0)).font(Menlo).color(simd_float4{1.0,1.0,1.0,1.0})},
        nodeHText{text(nodeHeightLabel()).fontSize(style::Size::pt(12.0)).font(Menlo).color(simd_float4{1.0,1.0,1.0,1.0})},
        nodeLocalBoxText{text(nodeLocalBoxLabel()).fontSize(style::Size::pt(12.0)).font(Menlo).color(simd_float4{1.0,1.0,1.0,1.0})},
        nodeScrollText{text(nodeScrollLabel()).fontSize(style::Size::pt(12.0)).font(Menlo).color(simd_float4{1.0,1.0,1.0,1.0})},
        nodeZIndexText{text(nodeZIndexLabel()).fontSize(style::Size::pt(12.0)).font(Menlo).color(simd_float4{1.0,1.0,1.0,1.0})},
        nodePathText{text(nodePathLabel()).fontSize(style::Size::pt(12.0)).font(Menlo).color(simd_float4{1.0,1.0,1.0,1.0})},
        frameStatsText{text("frame: waiting").fontSize(style::Size::pt(12.0)).font(Menlo).color(simd_float4{0.65,0.85,1.0,1.0})},
        phaseStatsText{text("phases: waiting").fontSize(style::Size::pt(11.0)).font(Menlo).color(simd_float4{0.94,0.96,1.0,1.0}).whiteSpace(style::WhiteSpace::PreWrap)},
        cacheStatsText{text("caches: waiting").fontSize(style::Size::pt(11.0)).font(Menlo).color(simd_float4{0.94,0.96,1.0,1.0}).whiteSpace(style::WhiteSpace::PreWrap)},
        renderStatsText{text("render: waiting").fontSize(style::Size::pt(11.0)).font(Menlo).color(simd_float4{0.94,0.96,1.0,1.0}).whiteSpace(style::WhiteSpace::PreWrap)},
        hitTestStatsText{text("hit tests: waiting").fontSize(style::Size::pt(11.0)).font(Menlo).color(simd_float4{0.94,0.96,1.0,1.0})},
        dirtyPhaseText{text("No selected node").fontSize(style::Size::pt(11.0)).font(Menlo).color(simd_float4{0.94,0.96,1.0,1.0}).whiteSpace(style::WhiteSpace::PreWrap)},
        mutationHistoryText{text("No recent mutations").fontSize(style::Size::pt(10.0)).font(Menlo).color(simd_float4{0.8,0.84,0.92,1.0}).whiteSpace(style::WhiteSpace::PreWrap)},
        treeViewText{text(treeViewDetails).fontSize(style::Size::pt(11.0)).font(Menlo).color(simd_float4{0.9,0.93,1.0,1.0}).whiteSpace(style::WhiteSpace::PreWrap)},
        selectionLabelText{text("None").fontSize(style::Size::pt(10.0)).font(Menlo).color(simd_float4{1.0,1.0,1.0,1.0}).pointerEvents(style::PointerEvents::None)},
        marginOverlayState{
            div()
                .color(simd_float4{0,0,0,0})
                .borderWidth(style::Size::px(1.0))
                .borderColor(simd_float4{1.0,0.55,0.15,0.95})
                .height(style::Size::px(0))
                .width(style::Size::px(0))
                .position(style::Position::Fixed)
                .pointerEvents(style::PointerEvents::None)
                .left(style::Size::px(-10000))
                .top(style::Size::px(-10000))
        },
        paddingOverlayState{
            div()
                .color(simd_float4{0,0,0,0})
                .borderWidth(style::Size::px(1.0))
                .borderColor(simd_float4{0.3,0.9,0.55,0.95})
                .height(style::Size::px(0))
                .width(style::Size::px(0))
                .position(style::Position::Fixed)
                .pointerEvents(style::PointerEvents::None)
                .left(style::Size::px(-10000))
                .top(style::Size::px(-10000))
        },
        contentOverlayState{
            div()
                .color(simd_float4{0,0,0,0})
                .borderWidth(style::Size::px(1.0))
                .borderColor(simd_float4{0.2,0.6,1.0,1.0})
                .height(style::Size::px(0))
                .width(style::Size::px(0))
                .position(style::Position::Fixed)
                .pointerEvents(style::PointerEvents::None)
                .left(style::Size::px(-10000))
                .top(style::Size::px(-10000))
        },
        selectionLabelState{
            div()
                .color(simd_float4{0.04,0.08,0.14,0.96})
                .padding(style::Size::px(4.0))
                .cornerRadius(style::Size::px(4.0))
                .position(style::Position::Fixed)
                .pointerEvents(style::PointerEvents::None)
                .left(style::Size::px(-10000))
                .top(style::Size::px(-10000))
            (
                selectionLabelText
            )
        },
        panelState{
            div()
                .color(simd_float4{0.035,0.045,0.07,0.8})
                .height(style::Size::px(720))
                .width(style::Size::px(420))
                .position(style::Position::Fixed)
                .padding(style::Size::px(14.0))
                .right(style::Size::px(12))
                .top(style::Size::px(12))
                .cornerRadius(style::Size::px(14.0))
                .borderWidth(style::Size::px(1.0))
                .borderColor(simd_float4{0.22,0.27,0.38,0.9})
                .overflow(style::Overflow::Scroll)
            (
                div()
                    .display(style::Display::Flex)
                    .flexDirection(FlexDirection::Col)
                    .flexGap(style::Size::px(12.0))
                (
                    div()
                        .display(style::Display::Flex)
                        .alignItems(style::AlignItems::Center)
                        .justifyContent(style::JustifyContent::SpaceBetween)
                    (
                        text("Inspector").fontSize(style::Size::pt(22.0)).font(ArialBold),
                        frameStatsText
                    ),
                    div()
                        .color(simd_float4{0.075,0.09,0.13,0.9})
                        .padding(style::Size::px(10.0))
                        .cornerRadius(style::Size::px(8.0))
                        .display(style::Display::Flex)
                        .flexDirection(FlexDirection::Col)
                        .flexGap(style::Size::px(4.0))
                    (
                        text("Renderer").fontSize(style::Size::pt(12.0)).font(ArialBold).color(simd_float4{0.55,0.78,1.0,1.0}),
                        phaseStatsText,
                        cacheStatsText,
                        renderStatsText,
                        hitTestStatsText
                    ),
                    div()
                        .color(simd_float4{0.075,0.09,0.13,0.9})
                        .padding(style::Size::px(10.0))
                        .cornerRadius(style::Size::px(8.0))
                        .display(style::Display::Flex)
                        .flexDirection(FlexDirection::Col)
                        .flexGap(style::Size::px(3.0))
                    (
                        text("Selected node").fontSize(style::Size::pt(12.0)).font(ArialBold).color(simd_float4{0.55,0.78,1.0,1.0}),
                        nodeIDText,
                        nodeTypeText,
                        nodeStyleText,
                        nodePositionText,
                        div().display(style::Display::Flex).flexGap(style::Size::px(12.0))
                        (
                            nodeWText,
                            nodeHText
                        ),
                        nodeLocalBoxText,
                        nodeScrollText,
                        nodeZIndexText,
                        nodePathText
                    ),
                    div()
                        .color(simd_float4{0.075,0.09,0.13,0.9})
                        .padding(style::Size::px(10.0))
                        .cornerRadius(style::Size::px(8.0))
                        .display(style::Display::Flex)
                        .flexDirection(FlexDirection::Col)
                        .flexGap(style::Size::px(4.0))
                    (
                        text("Dirty / recompute").fontSize(style::Size::pt(12.0)).font(ArialBold).color(simd_float4{0.55,0.78,1.0,1.0}),
                        dirtyPhaseText,
                        mutationHistoryText
                    ),
                    div()
                        .color(simd_float4{0.075,0.09,0.13,0.9})
                        .padding(style::Size::px(10.0))
                        .cornerRadius(style::Size::px(8.0))
                        .display(style::Display::Flex)
                        .flexDirection(FlexDirection::Col)
                        .flexGap(style::Size::px(4.0))
                    (
                        text("Hit stack  ·  Shift+Up/Down").fontSize(style::Size::pt(12.0)).font(ArialBold).color(simd_float4{0.55,0.78,1.0,1.0}),
                        hitStackText
                    ),
                    div()
                        .color(simd_float4{0.075,0.09,0.13,0.9})
                        .padding(style::Size::px(10.0))
                        .cornerRadius(style::Size::px(8.0))
                        .display(style::Display::Flex)
                        .flexDirection(FlexDirection::Col)
                        .flexGap(style::Size::px(4.0))
                    (
                        text("Tree").fontSize(style::Size::pt(12.0)).font(ArialBold).color(simd_float4{0.55,0.78,1.0,1.0}),
                        treeViewText
                    ),
                    div()
                        .display(style::Display::Flex)
                        .flexDirection(FlexDirection::Col)
                    (
                        text("Pointer").fontSize(style::Size::pt(11.0)).font(ArialBold).color(simd_float4{0.55,0.78,1.0,1.0}),
                        div().display(style::Display::Flex).flexGap(style::Size::px(12.0))
                        (
                            mouseXText,
                            mouseYText
                        )
                    )
                )
            )
        },
        visualizerState{
            div()
                .color(simd_float4{0,0,0,0})
                .height(style::Size::px(0))
                .width(style::Size::px(0))
                .position(style::Position::Fixed)
                .zIndex(DEBUG_Z_INDEX)
            (
                marginOverlayState,
                paddingOverlayState,
                contentOverlayState,
                selectionLabelState,
                panelState
            )
        }
    {
        inspectorNodeID = visualizerState.treeNode()->id;
    }

    void Inspector::observe(const Event& event) {
        lastEventType = event.type;

        switch (event.type) {
            case EventType::MouseDown:
            case EventType::MouseUp:
            case EventType::MouseMove:
            case EventType::MouseEnter:
            case EventType::MouseLeave:
            case EventType::Click:
                lastMouse = std::get<MousePayload>(event.payload);
                break;
            case EventType::KeyDown:
            case EventType::KeyUp:
                lastKeyboard = std::get<KeyboardPayload>(event.payload);
                break;
            case EventType::ScrollWheel:
                lastScroll = std::get<ScrollPayload>(event.payload);
                break;
            case EventType::Focus:
            case EventType::Blur:
                break;
        }

        auto& tree = Renderer::active().rootTree;
        auto hitNodes = tree.hitTestAll(getMouseState());
        hitNodes.erase(
            std::remove_if(hitNodes.begin(), hitNodes.end(), [this](auto* node) {
                for (auto* current = node; current; current = current->parent) {
                    if (current->id == inspectorNodeID) {
                        return true;
                    }
                }

                return false;
            }),
            hitNodes.end()
        );
        hitNodeCount = hitNodes.size();
        tree::TreeNode* selectedNode = nullptr;

        int cycleDirection = 0;
        if (event.type == EventType::KeyDown) {
            const auto& keyboard = std::get<KeyboardPayload>(event.payload);
            if (keyboard.modifiers.shift && keyboard.keyCode == KEY_CODE_UP) {
                cycleDirection = -1;
            }
            if (keyboard.modifiers.shift && keyboard.keyCode == KEY_CODE_DOWN) {
                cycleDirection = 1;
            }
        }

        if (hitNodes.empty()) {
            topNodeID = std::nullopt;
            htNodeID = std::nullopt;
            htNodeElementType = std::nullopt;
            htNodeDisplay = std::nullopt;
            htNodePosition = std::nullopt;
            htNodeOverflow = std::nullopt;
            htNodeX = std::nullopt;
            htNodeY = std::nullopt;
            htNodeW = std::nullopt;
            htNodeH = std::nullopt;
            htNodeLocalX = std::nullopt;
            htNodeLocalY = std::nullopt;
            htNodeLocalW = std::nullopt;
            htNodeLocalH = std::nullopt;
            htNodeScrollX = std::nullopt;
            htNodeScrollY = std::nullopt;
            htNodeZIndex = std::nullopt;
            htNodePath = std::nullopt;
            selectedHitIndex = 0;
        } else {
            auto newTopNodeID = hitNodes[0]->id;
            if (topNodeID != newTopNodeID) {
                topNodeID = newTopNodeID;
                selectedHitIndex = 0;
            }

            if (selectedHitIndex >= hitNodes.size()) {
                selectedHitIndex = 0;
            }

            if (cycleDirection > 0) {
                selectedHitIndex = (selectedHitIndex + 1) % hitNodes.size();
            } else if (cycleDirection < 0) {
                selectedHitIndex = selectedHitIndex == 0 ? hitNodes.size() - 1 : selectedHitIndex - 1;
            }

            auto htNode = hitNodes[selectedHitIndex];
            selectedNode = htNode;
            htNodeID = htNode->id;
            htNodeElementType = std::string(htNode->element->elementTypeName());
            htNodeDisplay = displayName(htNode->shared.display);
            htNodePosition = positionName(htNode->shared.position);
            htNodeOverflow = overflowName(htNode->shared.overflow);
            htNodeX = htNode->layout->computedBox.x;
            htNodeY = htNode->layout->computedBox.y;
            htNodeW = htNode->layout->computedBox.width;
            htNodeH = htNode->layout->computedBox.height;
            htNodeLocalX = htNode->layout->localComputedBox.x;
            htNodeLocalY = htNode->layout->localComputedBox.y;
            htNodeLocalW = htNode->layout->localComputedBox.width;
            htNodeLocalH = htNode->layout->localComputedBox.height;
            htNodeScrollX = float(htNode->scrollOffset.x);
            htNodeScrollY = float(htNode->scrollOffset.y);
            htNodeZIndex = htNode->globalZIndex;

            std::vector<tree::TreeNode*> path;
            for (auto* node = htNode; node; node = node->parent) {
                path.push_back(node);
            }
            std::reverse(path.begin(), path.end());

            std::string formattedPath;
            for (std::size_t i = 0; i < path.size(); ++i) {
                auto* node = path[i];
                auto line = std::format("{}#{}",
                    node->element->elementTypeName(),
                    node->id
                );
                formattedPath.append(line.begin(), line.end());
                if (i + 1 < path.size()) {
                    formattedPath.append(" > ");
                }
            }
            htNodePath = formattedPath;
        }

        hitStackDetails.clear();
        if (hitNodes.empty()) {
            hitStackDetails = "None";
        } else {
            for (std::size_t i = 0; i < hitNodes.size(); ++i) {
                auto* node = hitNodes[i];
                hitStackDetails += std::format(
                    "{} {}#{}  z:{}",
                    i == selectedHitIndex ? ">" : " ",
                    node->element->elementTypeName(),
                    node->id,
                    node->globalZIndex
                );
                if (i + 1 < hitNodes.size()) hitStackDetails += '\n';
            }
        }

        std::unordered_map<uint64_t, tree::TreeNode*> nodesById;
        treeViewDetails.clear();
        std::function<void(tree::TreeNode*, std::size_t)> appendTree =
            [&](tree::TreeNode* node, std::size_t depth) {
                if (!node || node->id == inspectorNodeID) return;

                nodesById.emplace(node->id, node);
                treeViewDetails.append(depth * 2, ' ');
                treeViewDetails += std::format(
                    "{}{}#{}",
                    htNodeID == node->id ? "> " : "  ",
                    node->element->elementTypeName(),
                    node->id
                );
                treeViewDetails += '\n';

                for (auto& child : node->children) {
                    appendTree(child.get(), depth + 1);
                }
            };
        appendTree(tree.getRoot(), 0);
        if (!treeViewDetails.empty()) treeViewDetails.pop_back();

        if constexpr (instrumentation::enabled) {
            auto& diagnostics = instrumentation::getDiagnostics();
            const auto& frame = diagnostics.latestFrame();

            frameStatsText.text(std::format(
                "frame {}  {:.2f} ms",
                frame.frameIndex,
                milliseconds(frame.elapsed)
            ));

            std::string phases;
            for (std::size_t i = 0; i < static_cast<std::size_t>(instrumentation::Phase::Count); ++i) {
                auto phase = static_cast<instrumentation::Phase>(i);
                const auto& phaseData = frame.phases[i];
                phases += std::format(
                    "{:<11} {:>6.2f} ms  {:>4} nodes",
                    phaseName(phase),
                    milliseconds(phaseData.elapsed),
                    phaseData.recomputedNodes
                );
                if (i + 1 < static_cast<std::size_t>(instrumentation::Phase::Count)) {
                    phases += '\n';
                }
            }
            phaseStatsText.text(phases);

            cacheStatsText.text(std::format(
                "render order  {} hit / {} miss  {:.2f} ms\n"
                "spec layout   {} hit / {} miss",
                frame.renderOrderCache.hits,
                frame.renderOrderCache.misses,
                milliseconds(frame.renderOrderCache.rebuildTime),
                frame.speculativeLayoutCache.hits,
                frame.speculativeLayoutCache.misses
            ));

            renderStatsText.text(std::format(
                "{} nodes  {} draws  {} atoms\n{} buffer writes  {:.1f} KiB",
                frame.render.nodesEncoded,
                frame.render.drawCalls,
                frame.render.atomsRendered,
                frame.render.bufferWrites,
                static_cast<double>(frame.render.bufferBytes) / 1024.0
            ));

            hitTestStatsText.text(std::format(
                "{} hit tests  {} examined  {} hits  {:.2f} ms",
                frame.hitTests.calls,
                frame.hitTests.nodesExamined,
                frame.hitTests.hits,
                milliseconds(frame.hitTests.elapsed)
            ));

            std::string dirtyDetails;
            if (selectedNode) {
                dirtyDetails = std::format(
                    "dirty self: 0x{:02x}  subtree: 0x{:02x}",
                    std::to_underlying(selectedNode->dirtySelf),
                    std::to_underlying(selectedNode->dirtySubtree)
                );

                if (auto found = diagnostics.nodes().find(selectedNode->id);
                    found != diagnostics.nodes().end()) {
                    constexpr std::array phasesToShow {
                        instrumentation::Phase::Measure,
                        instrumentation::Phase::Atomize,
                        instrumentation::Phase::Layout,
                        instrumentation::Phase::PostLayout,
                        instrumentation::Phase::Place,
                        instrumentation::Phase::Finalize
                    };
                    for (auto phase : phasesToShow) {
                        auto index = static_cast<std::size_t>(phase);
                        dirtyDetails += std::format(
                            "\n{:<11} {:<11} x{}",
                            phaseName(phase),
                            recomputeReasonName(found->second.lastRecomputeReasons[index]),
                            found->second.recomputeCounts[index]
                        );
                    }
                }
            } else {
                dirtyDetails = "No selected node";
            }
            dirtyPhaseText.text(dirtyDetails);

            std::string mutations;
            std::size_t mutationCount = 0;
            if (selectedNode) {
                for (auto frameIt = diagnostics.frames().rbegin();
                    frameIt != diagnostics.frames().rend() && mutationCount < 6;
                    ++frameIt) {
                    for (auto mutationIt = frameIt->mutations.rbegin();
                        mutationIt != frameIt->mutations.rend() && mutationCount < 6;
                        ++mutationIt) {
                        const auto& mutation = *mutationIt;
                        auto source = nodesById.find(mutation.sourceNodeId);
                        bool affectsSelection = mutation.sourceNodeId == selectedNode->id;
                        if (source != nodesById.end()) {
                            affectsSelection = affectsSelection ||
                                (hasPropagation(
                                    mutation.propagation,
                                    instrumentation::DirtyPropagation::Ancestors
                                ) && isAncestorOf(selectedNode, source->second)) ||
                                (hasPropagation(
                                    mutation.propagation,
                                    instrumentation::DirtyPropagation::Descendants
                                ) && isAncestorOf(source->second, selectedNode));
                        }
                        if (!affectsSelection) continue;

                        if (!mutations.empty()) mutations += '\n';
                        mutations += std::format(
                            "#{}  0x{:02x}→0x{:02x}  {}:{}",
                            mutation.sourceNodeId,
                            mutation.requestedDirtyBits,
                            mutation.effectiveSelfDirtyBits,
                            fileName(mutation.file),
                            mutation.line
                        );
                        mutationCount++;
                    }
                }
            }
            mutationHistoryText.text(
                mutations.empty() ? "No recent relevant mutations" : mutations
            );
        }

        updateSelectionOverlay(selectedNode);
    
        nodeIDText.text(nodeIdLabel());
        nodeTypeText.text(nodeTypeLabel());
        hitStackText.text(hitStackLabel());
        nodeStyleText.text(nodeStyleLabel());
        nodePositionText.text(nodePositionLabel());
        nodeWText.text(nodeWidthLabel());
        nodeHText.text(nodeHeightLabel());
        nodeLocalBoxText.text(nodeLocalBoxLabel());
        nodeScrollText.text(nodeScrollLabel());
        nodeZIndexText.text(nodeZIndexLabel());
        nodePathText.text(nodePathLabel());
        treeViewText.text(treeViewDetails);
        mouseXText.text(mouseXLabel());
        mouseYText.text(mouseYLabel());
        visualizerState.markDirty();
    }

    void Inspector::updateSelectionOverlay(tree::TreeNode* node) {
        if (!node || !node->layout.has_value()) {
            auto hide = [](auto& overlay) {
                overlay
                    .left(style::Size::px(-10000.0f))
                    .top(style::Size::px(-10000.0f))
                    .width(style::Size::px(0.0f))
                    .height(style::Size::px(0.0f));
            };
            hide(marginOverlayState);
            hide(paddingOverlayState);
            hide(contentOverlayState);
            selectionLabelState
                .left(style::Size::px(-10000.0f))
                .top(style::Size::px(-10000.0f));
            selectionLabelText.text("None");
            return;
        }

        const auto& layout = *node->layout;
        const auto& box = layout.computedBox;
        auto margins = node->preLayout.has_value()
            ? node->preLayout->resolvedMargins
            : layout::ResolvedMargins{};

        marginOverlayState
            .left(style::Size::px(box.x - margins.left))
            .top(style::Size::px(box.y - margins.top))
            .width(style::Size::px(box.width + margins.left + margins.right))
            .height(style::Size::px(box.height + margins.top + margins.bottom));

        paddingOverlayState
            .left(style::Size::px(box.x))
            .top(style::Size::px(box.y))
            .width(style::Size::px(box.width))
            .height(style::Size::px(box.height));

        const auto& padding = layout.resolvedPadding;
        contentOverlayState
            .left(style::Size::px(box.x + padding.left))
            .top(style::Size::px(box.y + padding.top))
            .width(style::Size::px(std::max(0.0f, box.width - padding.left - padding.right)))
            .height(style::Size::px(std::max(0.0f, box.height - padding.top - padding.bottom)));

        selectionLabelText.text(std::format(
            "{}#{}  {:.0f}×{:.0f}",
            node->element->elementTypeName(),
            node->id,
            box.width,
            box.height
        ));
        selectionLabelState
            .left(style::Size::px(box.x))
            .top(style::Size::px(std::max(0.0f, box.y - 22.0f)));
    }

    // store state
    // add an element with high z index, controlled by cursor, scroll for sorting by z-index or some other control?
    // 

    simd_float2 Inspector::getMouseState() {
        if (lastMouse.has_value()) {
            return simd_float2{lastMouse->position.x, lastMouse->position.y};
        }

        return simd_float2{0.0,0.0};
    }

    std::string Inspector::nodeIdLabel() {
        auto cnid = htNodeID.has_value() ? std::format("currentNodeId: {}", *htNodeID) : std::format("currentNodeId: None");
        return cnid;
    }

    std::string Inspector::nodeTypeLabel() {
        auto type = htNodeElementType.has_value() ? std::format("type: {}", *htNodeElementType) : std::format("type: None");
        return type;
    }

    std::string Inspector::hitStackLabel() {
        return hitStackDetails;
    }

    std::string Inspector::nodeStyleLabel() {
        auto style = htNodeDisplay.has_value() && htNodePosition.has_value() && htNodeOverflow.has_value()
            ? std::format("display: {} posMode: {} overflow: {}", *htNodeDisplay, *htNodePosition, *htNodeOverflow)
            : std::format("style: None");
        return style;
    }

    std::string Inspector::nodePositionLabel() {
        auto position = htNodeX.has_value() && htNodeY.has_value()
            ? std::format("global: {}, {}", *htNodeX, *htNodeY)
            : std::format("global: None");
        return position;
    }

    std::string Inspector::nodeWidthLabel() {
        auto width = htNodeW.has_value() ? std::format("w: {}", *htNodeW) : std::format("w: None");
        return width;
    }

    std::string Inspector::nodeHeightLabel() {
        auto height = htNodeH.has_value() ? std::format("h: {}", *htNodeH) : std::format("h: None");
        return height;
    }

    std::string Inspector::nodeLocalBoxLabel() {
        auto box = htNodeLocalX.has_value() && htNodeLocalY.has_value() && htNodeLocalW.has_value() && htNodeLocalH.has_value()
            ? std::format("local: {}, {} {}x{}", *htNodeLocalX, *htNodeLocalY, *htNodeLocalW, *htNodeLocalH)
            : std::format("local: None");
        return box;
    }

    std::string Inspector::nodeScrollLabel() {
        auto scroll = htNodeScrollX.has_value() && htNodeScrollY.has_value()
            ? std::format("scroll: {}, {}", *htNodeScrollX, *htNodeScrollY)
            : std::format("scroll: None");
        return scroll;
    }

    std::string Inspector::nodeZIndexLabel() {
        auto zIndex = htNodeZIndex.has_value() ? std::format("zIndex: {}", *htNodeZIndex) : std::format("zIndex: None");
        return zIndex;
    }

    std::string Inspector::nodePathLabel() {
        return htNodePath.value_or("path: None");
    }

    std::string Inspector::mouseXLabel() {
        auto [mouseX, mouseY] = getMouseState();
        auto mx = std::format("x: {}", float(mouseX));
        return mx;
    }

    std::string Inspector::mouseYLabel() {
        auto [mouseX, mouseY] = getMouseState();
        auto my = std::format("y: {}", float(mouseY));
        return my;
    }

    NodeBuilder<Div<DivStorage>, DivProcessor<DivStorage, DivUniforms>> Inspector::visualizer() {
        return this->visualizerState;
    }
};
