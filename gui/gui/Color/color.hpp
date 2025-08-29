//
//  color.hpp
//  gui
//
//  Created by Taanish Reja on 8/28/25.
//

#include <variant>

struct RGB {
    
    
    
    
    
    void get();
    
    float normalizedR;
    float normalizedG;
    float normalizedB;
}

struct Hex {
    Hex(int hexCode);
    Hex(const std::string& hexCode);
    
    int parser(const std::string& hexCode);
    
    void get();

    unsigned int hex;
}

using Color = std::variant<RGB, Hex>;
