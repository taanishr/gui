#include "svg.hpp"
#include "sizing.hpp"

elements::SVGAsset::~SVGAsset() {
    if (tree) resvg_tree_destroy(tree);
}

std::shared_ptr<elements::SVGAsset> elements::SVGCache::retrieve(const std::string& path) {
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

    auto asset = std::make_shared<SVGAsset>();
    asset->path = path;

    auto* options = resvg_options_create();
    int32_t error = resvg_parse_tree_from_file(path.c_str(), options, &asset->tree);
    resvg_options_destroy(options);

    if (error != RESVG_OK || !asset->tree) {
        std::println("resvg: failed to parse {}, error={}", path, error);
        asset->tree = nullptr;
    } else {
        auto size = resvg_get_image_size(asset->tree);
        asset->intrinsicSize = {size.width, size.height};
    }

    assets.emplace(path, asset);
    return asset;
}

elements::SVGDescriptor::SVGDescriptor():
    path{}
{}
