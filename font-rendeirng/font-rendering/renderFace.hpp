//
//  renderFace.hpp
//  font-rendeirng
//
//  Created by Taanish Reja on 7/25/25.
//

#include <iostream>
#include <iomanip>
#include <cmath>
#include "freetype.hpp"
#include "metal_imports.hpp"
#include "bezier.hpp"

std::vector<simd_float2> renderPoints(char ch, FT_Library ft, std::string_view fontPath,  int resolution = 25);
