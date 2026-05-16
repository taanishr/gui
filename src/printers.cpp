//
//  printers.cpp
//  gui
//
//  Created by Taanish Reja on 8/20/25.
//

#include "printers.hpp"

void printPoint(const simd_float2& pt, char delim) {
    std::print("({},{}){}", pt.x, pt.y, delim);
}
