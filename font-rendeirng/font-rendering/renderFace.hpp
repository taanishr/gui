//
//  renderFace.hpp
//  font-rendeirng
//
//  Created by Taanish Reja on 7/25/25.
//

#include <iostream>
#include <iomanip>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <cmath>


std::vector<simd_float2> renderPoints(char ch, FT_Library ft, std::string_view fontPath);
