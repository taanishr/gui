//
//  MTKTextureLoader.cpp
//  MTKTextureBindings
//
//  Created by Taanish Reja on 1/21/25.
//

#include "MTKTexture_loader.hpp"
#include <MTK_Extensions-Swift.h>

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

    MTL::Texture* createTexture(const MTKTextureLoader& textureLoader,
                                const std::string& filepath)
    {
        
        MTL::Texture* texture = static_cast<MTL::Texture*>
            (MTK_Extensions::createTexture(
                                        textureLoader.getTextureLoader()->get(),
                                        filepath.c_str()
            )
        );
        return texture;
    }
}
