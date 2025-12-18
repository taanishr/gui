//
//  buffer_allocator.cpp
//  gui
//
//  Created by Taanish Reja on 11/23/25.
//

#include "buffer_allocator.hpp"

DrawableBuffer::DrawableBuffer(MTL::Device* device, uint64_t bufferId, uint64_t size):
    bufferId{bufferId}
{
    buffer = device->newBuffer(size, MTL::ResourceStorageModeShared);
}

BufferHandle DrawableBuffer::handle() {
    return bufferId;
}

MTL::Buffer* DrawableBuffer::get() {
    return buffer;
}

DrawableBuffer::DrawableBuffer(DrawableBuffer&& other) {
    this->buffer = other.buffer;
    this->bufferId = other.bufferId;
    
    other.bufferId = -1;
    other.buffer = nullptr;
}

DrawableBuffer& DrawableBuffer::operator=(DrawableBuffer&& other) {
    this->buffer = other.buffer;
    this->bufferId = other.bufferId;
    
    other.bufferId = -1;
    other.buffer = nullptr;
    
    return *this;
}

DrawableBuffer::~DrawableBuffer() {
    if (buffer) {
        buffer->release();
    }
}

DrawableBufferAllocator::DrawableBufferAllocator(MTL::Device* device):
    device{device}
{}

DrawableBuffer DrawableBufferAllocator::allocate(size_t size) {
    DrawableBuffer newBuffer {device, nextId, size};
    nextId++;
    return newBuffer;
}

void DrawableBufferAllocator::resize(DrawableBuffer db, size_t newSize) {
    auto rawBuffer = db.buffer;
    
    unsigned long oldSize = rawBuffer->length();

    if (newSize < oldSize)
        return;

    MTL::Buffer* newBuffer = device->newBuffer(std::max(oldSize*2, newSize), MTL::StorageModeShared);
    
    if (!newBuffer) return;
    
    rawBuffer->release();
    rawBuffer = newBuffer;
}
