#include "div.hpp"
#include "new_arch.hpp"

NewArch::DivDescriptor::DivDescriptor():
    display{Display::Block},
    position{Position::Relative},
    top{0},
    left{0},
    cornerRadius{0}
{}