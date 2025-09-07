//
//  node_builder.cpp
//  gui
//
//  Created by Taanish Reja on 8/26/25.
//

#include "node_builder.hpp"


NodeBuilder<Shell, ShellLayout> div(float cornerRadius, simd_float4 color) {
    auto& renderer = Renderer::active();
    auto& tree = renderer.renderTree;
    auto parent = tree.root.get();
    return NodeBuilder<Shell, ShellLayout>(tree, parent, std::make_unique<Shell>(renderer, color, cornerRadius));
}

NodeBuilder<Text> text(const std::string& text, float fontSize, float x, float y) {
    auto& renderer = Renderer::active();
    auto& tree = renderer.renderTree;
    auto parent = tree.root.get();
    auto nb = NodeBuilder(tree, parent, std::make_unique<Text>(renderer, x, y, fontSize, simd_float4{1,1,1,1}));
    nb.node->drawable->setText(text);
    return nb;
}

NodeBuilder<ImageDrawable, ImageLayout> image(const std::string& path) {
    auto& renderer = Renderer::active();
    auto& tree = renderer.renderTree;
    auto parent = tree.root.get();
    auto nb = NodeBuilder<ImageDrawable, ImageLayout>(tree, parent, std::make_unique<ImageDrawable>(renderer, path));
    return nb;
}
