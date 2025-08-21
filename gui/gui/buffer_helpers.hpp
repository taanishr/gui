//
//  buffer_helpers.hpp
//  gui
//
//  Created by Taanish Reja on 8/21/25.
//

#pragma once

#include "metal_imports.hpp"

void resizeBuffer(MTL::Device* device, MTL::Buffer*& buffer, unsigned long numBytes);
