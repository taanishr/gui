//
//  inputState.hpp
//  font-rendering
//
//  Created by Taanish Reja on 8/15/25.
//

#ifndef INPUT_STATE_H
#define INPUT_STATE_H
#include <string>
#include "text.hpp"

struct SelectedString {
    static void removeChar();
    static void addChar(char ch);
    
    static Text* textBlock;
    static std::string selectedString;
};

//extern std::string selectedString;

#endif
