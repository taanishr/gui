//
//  buffer_allocator.cpp
//  gui
//
//  Created by Taanish Reja on 11/23/25.
//

#include "buffer_allocator.hpp"

DrawableBuffer::DrawableBuffer(MTL::Device* device, unsigned long bufferId, unsigned long size):
    bufferId{bufferId}
{
    buffer = device->newBuffer(size, MTL::ResourceStorageModeShared);
}

DrawableBuffer::~DrawableBuffer() {
    buffer->release();
}

DrawableBufferAllocator::DrawableBufferAllocator(MTL::Device* device):
    device{device},
    buffers{}
{}

BufferHandle DrawableBufferAllocator::allocate(unsigned long size) {
    buffers[nextId] = std::make_unique<DrawableBuffer>(device, nextId, size);
    return nextId++;
}

void DrawableBufferAllocator::resize(BufferHandle handle, unsigned long newSize) {
    auto it = buffers.find(handle);
    
    if (it != buffers.end()) {
        auto rawBuffer = it->second->buffer;
        
        unsigned long oldSize = rawBuffer->length();

        if (newSize < oldSize)
            return;

        MTL::Buffer* newBuffer = device->newBuffer(std::max(oldSize*2, newSize), MTL::StorageModeShared);
        
        if (!newBuffer) return;
        
        rawBuffer->release();
        rawBuffer = newBuffer;
    }
}

MTL::Buffer* DrawableBufferAllocator::get(BufferHandle handle) {
    auto it = buffers.find(handle);
    
    if (it != buffers.end()) {
        auto db = it->second.get(); // grab reference; otherwise destructor crashes. Change to shared ptr?
        return db->buffer;
    }
    
    return nullptr;
}
