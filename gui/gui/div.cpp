#include "div.hpp"
#include "new_arch.hpp"
#include <optional>

NewArch::DivDescriptor::DivDescriptor()
    : width{std::nullopt}
    , height{std::nullopt}
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
    , top{std::nullopt}
    , right{std::nullopt}
    , left{std::nullopt}
    , bottom{std::nullopt}
    , display{Display::Block}
    , position{Position::Static}
{
}