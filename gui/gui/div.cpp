#include "div.hpp"
#include "new_arch.hpp"

NewArch::DivDescriptor::DivDescriptor()
    : width{Size::px(0)}
    , height{Size::px(0)}
    , color{0.0f, 0.0f, 0.0f, 1.0f}
    , cornerRadius{Size::px(0)}
    , borderWidth{Size::px(0)}
    , borderColor{0.0f, 0.0f, 0.0f, 1.0f}
    , padding{Size::px(0.0f)}
    , paddingLeft{std::nullopt}
    , paddingRight{std::nullopt}
    , paddingTop{std::nullopt}
    , paddingBottom{std::nullopt}
    , margin{Size::px(0)}
    , marginLeft{std::nullopt}
    , marginRight{std::nullopt}
    , marginTop{std::nullopt}
    , marginBottom{std::nullopt}
    , top{Size::px(0.0f)}
    , left{Size::px(0.0f)}
    , display{Display::Block}
    , position{Position::Relative}
{
}