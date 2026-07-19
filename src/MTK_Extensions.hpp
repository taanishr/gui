#pragma once

#include <cstdint>

namespace MTK_Extensions {
    extern "C" void* createMTKTextureLoader(void* devicePtr);
    extern "C" void* releaseMTKTextureLoader(void* loaderPtr);
    extern "C" void* createDownsampledTexture(
        void* loaderPtr,
        const char* filePath,
        uint32_t targetPixelWidth,
        uint32_t targetPixelHeight
    );
};
