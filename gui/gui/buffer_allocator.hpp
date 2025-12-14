//
//  buffer_allocator.hpp
//  gui
//
//  Created by Taanish Reja on 11/23/25.
//

#include "metal_imports.hpp"
#include <print>

using BufferHandle = unsigned long;

struct DrawableBuffer {
    BufferHandle bufferId;
    MTL::Buffer* buffer;
    
    DrawableBuffer(MTL::Device* device, unsigned long bufferId, unsigned long size);
    DrawableBuffer(const DrawableBuffer&) = delete;
    ~DrawableBuffer();
};

struct DrawableBufferAllocator{
    BufferHandle nextId;
    std::unordered_map<BufferHandle, std::unique_ptr<DrawableBuffer>> buffers;
    MTL::Device* device;
    
    DrawableBufferAllocator(MTL::Device* device);
    
    BufferHandle allocate(unsigned long size);
    void resize(BufferHandle handle, unsigned long newSize);
    MTL::Buffer* get(BufferHandle handle);
};

