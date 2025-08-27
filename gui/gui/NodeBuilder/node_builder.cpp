//
//  node_builder.cpp
//  gui
//
//  Created by Taanish Reja on 8/26/25.
//

#include "node_builder.hpp"


NodeBuilder<Shell> div(Renderer& renderer, float w, float h, float cornerRadius, float x, float y, simd_float4 color) {
    auto& tree = renderer.renderTree;
    auto parent = tree.root.get();
    return NodeBuilder(tree, parent, std::make_unique<Shell>(renderer, w, h, x, y, color, cornerRadius));
}

NodeBuilder<Text> text(Renderer& renderer, const std::string& text, float fontSize, float x, float y) {
    auto& tree = renderer.renderTree;
    auto parent = tree.root.get();
    auto nb = NodeBuilder(tree, parent, std::make_unique<Text>(renderer, x, y, fontSize, simd_float4{1,1,1,1}));
    nb.node->drawable->setText(text);
    return nb;
}
