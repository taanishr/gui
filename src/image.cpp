#include "image.hpp"
#include "sizing.hpp"

std::shared_ptr<elements::ImageAsset> elements::ImageCache::retrieve(
    const std::string& path,
    const MTKTextures::MTKTextureLoader& textureLoader
) {
    {
        std::shared_lock lock(mutex);
        if (auto found = assets.find(path); found != assets.end()) {
            return found->second;
        }
    }

    std::unique_lock lock(mutex);
    if (auto found = assets.find(path); found != assets.end()) {
        return found->second;
    }

    auto asset = std::make_shared<ImageAsset>();
    asset->path = path;
    asset->texture = NS::TransferPtr(
        MTKTextures::createTexture(textureLoader, path)
    );

    if (asset->texture) {
        asset->intrinsicSize = {
            static_cast<float>(asset->texture->width()),
            static_cast<float>(asset->texture->height())
        };
    }

    assets.emplace(path, asset);
    return asset;
}

elements::ImageDescriptor::ImageDescriptor():
    path{}
{}
