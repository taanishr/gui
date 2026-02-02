#include "text.hpp"
#include "sizing.hpp"

NewArch::TextDescriptor::TextDescriptor():
    text{},
    font{},
    color{0.0f, 0.0f, 0.0f, 1.0f},
    fontSize{Size::pt(12.0f)},
    display{Display::Inline},
    position{Position::Relative},
    margin{Size::px(0)},
    marginLeft{std::nullopt},
    marginRight{std::nullopt},
    marginTop{std::nullopt},
    marginBottom{std::nullopt}
{}