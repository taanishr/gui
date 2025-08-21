//
//  buffer_helpers.cpp
//  gui
//
//  Created by Taanish Reja on 8/21/25.
//

#include "buffer_helpers.hpp"

void resizeBuffer(MTL::Device* device, MTL::Buffer*& buffer, unsigned long numBytes) {
    unsigned long oldLength = buffer->length();

    if (numBytes < oldLength)
        return;

    unsigned long newLength = std::max(oldLength*2, numBytes);

    MTL::Buffer* newBuffer = device->newBuffer(newLength, MTL::StorageModeShared);
    buffer->release();
    buffer = newBuffer;
}
