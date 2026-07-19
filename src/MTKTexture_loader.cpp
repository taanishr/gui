//
//  MTKTextureLoader.cpp
//  MTKTextureBindings
//
//  Created by Taanish Reja on 1/21/25.
//

#include "MTKTexture_loader.hpp"
#include "MTK_Extensions.hpp"

namespace MTKTextures {
    MTKTextureLoader::MTKTextureLoader(MTL::Device* device):
    m_textureLoader{std::make_shared<SwiftObject>
        (MTK_Extensions::createMTKTextureLoader(static_cast<void*>(device)),
         MTK_Extensions::releaseMTKTextureLoader)
    }
    {}

    MTKTextureLoader::~MTKTextureLoader() = default;

    SwiftObject* MTKTextureLoader::getTextureLoader() const
    {
        return m_textureLoader.get();
    }

    MTL::Texture* createDownsampledTexture(
        const MTKTextureLoader& textureLoader,
        const std::string& filepath,
        uint32_t targetPixelWidth,
        uint32_t targetPixelHeight
    ) {
        return static_cast<MTL::Texture*>(
            MTK_Extensions::createDownsampledTexture(
                textureLoader.getTextureLoader()->get(),
                filepath.c_str(),
                targetPixelWidth,
                targetPixelHeight
            )
        );
    }
}
