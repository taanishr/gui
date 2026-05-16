//
//  MTKTextureLoader.h
//  MTKTextureBindings
//
//  Created by Taanish Reja on 1/21/25.
//

#pragma once

#include "swift_object.hpp"
#include "metal_imports.hpp"

namespace MTKTextures {
    class MTKTextureLoader {
    public:
        MTKTextureLoader(MTL::Device*);
        ~MTKTextureLoader();
        
        SwiftObject* getTextureLoader() const;
    private:
        std::shared_ptr<SwiftObject> m_textureLoader;
    };

    MTL::Texture* createTexture(const MTKTextureLoader& textureLoader, 
                                const std::string& filepath);

}

