#include "image.hpp"
#include "sizing.hpp"

NewArch::ImageDescriptor::ImageDescriptor():
    width{Size::px(0)},
    height{Size::px(0)},
    path{},
    cornerRadius{},
    borderWidth{},
    borderColor{0.0f, 0.0f, 0.0f, 1.0f},
    display{Display::Block},
    position{Position::Static},
    margin{Size::px(0)},
    marginLeft{std::nullopt},
    marginRight{std::nullopt},
    marginTop{std::nullopt},
    marginBottom{std::nullopt}
{}