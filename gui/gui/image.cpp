#include "image.hpp"
#include "sizing.hpp"

NewArch::ImageDescriptor::ImageDescriptor():
    width{Size::px(0)},
    height{Size::px(0)},
    path{},
    cornerRadius{0.0f},
    borderWidth{0.0f},
    borderColor{0.0f, 0.0f, 0.0f, 1.0f},
    display{Display::Block},
    position{Position::Relative},
    margin{Size::px(0)},
    marginLeft{std::nullopt},
    marginRight{std::nullopt},
    marginTop{std::nullopt},
    marginBottom{std::nullopt}
{}