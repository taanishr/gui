//
//  LayoutBox.cpp
//  gui
//
//  Created by Taanish Reja on 9/4/25.
//

#include "LayoutBox.hpp"

LayoutBox::LayoutBox():
    positioningType{PositioningType::Absolute},
    width{0},
    height{0},
    minWidth{0},
    maxWidth{0},
    minHeight{0},
    maxHeight{0},
    offset{0,0},
    globalPosition{0,0}
{}
