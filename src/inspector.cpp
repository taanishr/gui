#include "inspector.hpp"
#include "context_manager.hpp"
#include "events.hpp"
#include "new_arch.hpp"
#include "sizing.hpp"
#include <algorithm>
#include <print>
#include <string>
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
        visualizerState{
            div()
                .color(simd_float4{0,0,0,0.5})
                .height(style::Size::px(320))
                .width(style::Size::px(280))
                .position(style::Position::Fixed)
                .zIndex(DEBUG_Z_INDEX)
                .padding(style::Size::px(12.0))
                .right(style::Size::px(0))
                .top(style::Size::px(0))
                .margin(style::Size::px(12.0))
                .cornerRadius(style::Size::px(12.0))
                .overflow(style::Overflow::Scroll)
            (
                text(U"Inspector").fontSize(style::Size::pt(24.0)).font(ArialBold),
                div()
                    .display(style::Display::Flex)
                    .flexDirection(FlexDirection::Col)
                    .flexGap(style::Size::px(10.0))
                (
                    div()
                        .display(style::Display::Flex)
                        .flexDirection(FlexDirection::Col)
                    (
                        text(U"Mouse").fontSize(style::Size::pt(12.0)).font(ArialBold),
                        div().display(style::Display::Flex).flexGap(style::Size::px(12.0))
                        (
                            mouseXText,
                            mouseYText
                        )
                    ),
                    div()
                        .display(style::Display::Flex)
                        .flexDirection(FlexDirection::Col)
                    (
                        text(U"Node").fontSize(style::Size::pt(12.0)).font(ArialBold),
                        hitStackText,
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
                        nodeZIndexText
                    ),
                    div()
                        .display(style::Display::Flex)
                        .flexDirection(FlexDirection::Col)
                    (
                        text(U"Controls").fontSize(style::Size::pt(12.0)).font(ArialBold),
                        text(U"Shift+Up/Down cycle nodes").fontSize(style::Size::pt(12.0)).font(Menlo)
                    ),
                    div()
                        .display(style::Display::Flex)
                        .flexDirection(FlexDirection::Col)
                    (
                        text(U"Path").fontSize(style::Size::pt(12.0)).font(ArialBold),
                        nodePathText
                    )
                )
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

            std::u32string formattedPath;
            for (std::size_t i = 0; i < path.size(); ++i) {
                auto* node = path[i];
                auto line = std::format("{}#{}",
                    node->element->elementTypeName(),
                    node->id
                );
                formattedPath.append(line.begin(), line.end());
                if (i + 1 < path.size()) {
                    formattedPath.append(U" > ");
                }
            }
            htNodePath = formattedPath;
        }
    
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
        mouseXText.text(mouseXLabel());
        mouseYText.text(mouseYLabel());
        visualizerState.markDirty();
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

    std::u32string Inspector::nodeIdLabel() {
        auto cnid = htNodeID.has_value() ? std::format("currentNodeId: {}", *htNodeID) : std::format("currentNodeId: None");
        return std::u32string(cnid.begin(), cnid.end());
    }

    std::u32string Inspector::nodeTypeLabel() {
        auto type = htNodeElementType.has_value() ? std::format("type: {}", *htNodeElementType) : std::format("type: None");
        return std::u32string(type.begin(), type.end());
    }

    std::u32string Inspector::hitStackLabel() {
        auto hit = hitNodeCount > 0 ? std::format("hit: {} / {}", selectedHitIndex + 1, hitNodeCount) : std::format("hit: None");
        return std::u32string(hit.begin(), hit.end());
    }

    std::u32string Inspector::nodeStyleLabel() {
        auto style = htNodeDisplay.has_value() && htNodePosition.has_value() && htNodeOverflow.has_value()
            ? std::format("display: {} posMode: {} overflow: {}", *htNodeDisplay, *htNodePosition, *htNodeOverflow)
            : std::format("style: None");
        return std::u32string(style.begin(), style.end());
    }

    std::u32string Inspector::nodePositionLabel() {
        auto position = htNodeX.has_value() && htNodeY.has_value()
            ? std::format("global: {}, {}", *htNodeX, *htNodeY)
            : std::format("global: None");
        return std::u32string(position.begin(), position.end());
    }

    std::u32string Inspector::nodeWidthLabel() {
        auto width = htNodeW.has_value() ? std::format("w: {}", *htNodeW) : std::format("w: None");
        return std::u32string(width.begin(), width.end());
    }

    std::u32string Inspector::nodeHeightLabel() {
        auto height = htNodeH.has_value() ? std::format("h: {}", *htNodeH) : std::format("h: None");
        return std::u32string(height.begin(), height.end());
    }

    std::u32string Inspector::nodeLocalBoxLabel() {
        auto box = htNodeLocalX.has_value() && htNodeLocalY.has_value() && htNodeLocalW.has_value() && htNodeLocalH.has_value()
            ? std::format("local: {}, {} {}x{}", *htNodeLocalX, *htNodeLocalY, *htNodeLocalW, *htNodeLocalH)
            : std::format("local: None");
        return std::u32string(box.begin(), box.end());
    }

    std::u32string Inspector::nodeScrollLabel() {
        auto scroll = htNodeScrollX.has_value() && htNodeScrollY.has_value()
            ? std::format("scroll: {}, {}", *htNodeScrollX, *htNodeScrollY)
            : std::format("scroll: None");
        return std::u32string(scroll.begin(), scroll.end());
    }

    std::u32string Inspector::nodeZIndexLabel() {
        auto zIndex = htNodeZIndex.has_value() ? std::format("zIndex: {}", *htNodeZIndex) : std::format("zIndex: None");
        return std::u32string(zIndex.begin(), zIndex.end());
    }

    std::u32string Inspector::nodePathLabel() {
        return htNodePath.value_or(U"path: None");
    }

    std::u32string Inspector::mouseXLabel() {
        auto [mouseX, mouseY] = getMouseState();
        auto mx = std::format("x: {}", float(mouseX));
        return std::u32string(mx.begin(), mx.end());
    }

    std::u32string Inspector::mouseYLabel() {
        auto [mouseX, mouseY] = getMouseState();
        auto my = std::format("y: {}", float(mouseY));
        return std::u32string(my.begin(), my.end());
    }

    NodeBuilder<Div<DivStorage>, DivProcessor<DivStorage, DivUniforms>> Inspector::visualizer() {
        return this->visualizerState;
    }
};
