//
//  buffer_allocator.hpp
//  gui
//
//  Created by Taanish Reja on 11/23/25.
//

#pragma once
#include "metal_imports.hpp"
#include <print>

using BufferHandle = uint64_t;

struct DrawableBuffer {
    BufferHandle bufferId;
    MTL::Buffer* buffer;
    
    BufferHandle handle();
    MTL::Buffer* get();
    void resize(unsigned long newSize);
    
    DrawableBuffer(MTL::Device* device, uint64_t bufferId, uint64_t size);
    DrawableBuffer(DrawableBuffer&&);
    DrawableBuffer& operator=(DrawableBuffer&& other);
    
    DrawableBuffer(const DrawableBuffer&) = delete;
    DrawableBuffer& operator=(DrawableBuffer& other) = delete;

    ~DrawableBuffer();
};

struct DrawableBufferAllocator{
    BufferHandle nextId;
    MTL::Device* device;
    
    DrawableBufferAllocator(MTL::Device* device);
    
    DrawableBuffer allocate(size_t size);
    void resize(DrawableBuffer buffer, size_t newSize);
};

