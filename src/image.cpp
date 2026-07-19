#include "image.hpp"
#include "sizing.hpp"

std::shared_ptr<elements::ImageAsset> elements::ImageCache::retrieve(const std::string& path) {
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

    assets.emplace(path, asset);
    return asset;
}

uint32_t elements::ImageCache::quantize(float value) {
    uint32_t bucket = static_cast<uint32_t>((value + BucketSize / 2.0f) / BucketSize) * BucketSize;
    return std::max(bucket, BucketSize);
}

NS::SharedPtr<MTL::Texture> elements::ImageCache::retrieveTexture(
    const std::shared_ptr<ImageAsset>& asset,
    ImageRenditionKey renditionKey,
    const MTKTextures::MTKTextureLoader& textureLoader
) {
    std::lock_guard lock(asset->renditionMutex);
    if (auto found = asset->renditions.find(renditionKey); found != asset->renditions.end()) {
        return found->second;
    }

    auto texture = NS::TransferPtr(
        MTKTextures::createDownsampledTexture(
            textureLoader,
            asset->path,
            renditionKey.first,
            renditionKey.second
        )
    );

    auto [rendition, _] = asset->renditions.emplace(renditionKey, texture);
    return rendition->second;
}

elements::ImageDescriptor::ImageDescriptor():
    path{}
{}
