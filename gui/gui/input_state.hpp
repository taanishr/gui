//
//  inputState.hpp
//  font-rendering
//
//  Created by Taanish Reja on 8/15/25.
//

#pragma once
#include <string>
#include "ui.hpp"

struct SelectedString {
    static void removeChar();
    static void addChar(char ch);
    
    static Text* textBlock;
    static std::string selectedString;
};
