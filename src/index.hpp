//
//  index.hpp
//  gui
//
//  Created by Taanish Reja on 9/2/25.
//

#include "new_arch.hpp"
#include "node_builder.hpp"

namespace gui {
    using style::Size;
    using style::Display;
    using style::Position;
    using style::FlexDirection;
    using style::JustifyContent;
    using style::AlignItems;
    using style::AlignSelf;
    using style::AlignContent;
    using style::FlexWrap;
    using style::Overflow;

    using elements::div;
    using elements::text;
    using elements::image;
    using elements::svg;

    using tree::RenderTree;
    using runtime::UIContext;
    using runtime::ContextManager;
}

auto index() -> void;
