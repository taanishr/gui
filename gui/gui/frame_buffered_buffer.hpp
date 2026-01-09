#include "gui/buffer_allocator.hpp"
#include <cstdint>
template <typename T>
struct FrameBufferedBuffer {
    FrameBufferedBuffer<T>(DrawableBufferAllocator& allocator, size_t size, uint64_t numFrames):
        allocator{allocator},
        numFrames{numFrames}
    {
        for (int i = 0; i < numFrames; ++i) {
            buffers.push_back(DrawableBuffer{allocator.allocate(size)});
        }
    }

    void write(uint64_t frameIndex, T* data, size_t length, size_t offset = 0) {
        auto writingIndex = frameIndex % numFrames;
        auto& buffer = buffers[writingIndex];
        auto rawBuffer = buffer.get();

        if (length > rawBuffer->length()) {
            allocator.resize(buffer, length);
        }

        auto rawBufferContents = reinterpret_cast<std::byte*>(rawBuffer->contents());

        std::memcpy(rawBufferContents + offset, data, length);
    }

    BufferHandle getBufferHandle(uint64_t frameIndex) {
        return buffers[frameIndex % numFrames].handle();   
    }

    MTL::Buffer* getBuffer(uint64_t frameIndex) {
        return buffers[frameIndex % numFrames].get();
    }

    std::vector<DrawableBuffer> buffers;
    DrawableBufferAllocator& allocator;
    uint64_t numFrames;
};