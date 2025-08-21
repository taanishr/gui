//
//  inputState.cpp
//  font-rendering
//
//  Created by Taanish Reja on 8/15/25.
//

#include "input_state.hpp"

//std::string selectedString = "";

Text* SelectedString::textBlock = nullptr;
std::string SelectedString::selectedString;

void SelectedString::removeChar() {
    if (selectedString.length() == 0)
        return;
    
    selectedString.pop_back();
    
    if (textBlock) {
        textBlock->setText(selectedString);
    }
}

void SelectedString::addChar(char ch) {
    selectedString += ch;
    
    if (textBlock) {
        textBlock->setText(selectedString);
    }
}
