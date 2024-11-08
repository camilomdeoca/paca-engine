#include <ResourceFileFormats.hpp>

#include "../../ModelConverter.hpp"
#include <serializers/Serializer.hpp>

#include <print>
#include <filesystem>

int main (int argc, char *argv[]) {
    size_t size;
    std::string path = std::filesystem::path(__FILE__).parent_path() / "plane/plane.gltf";
    std::println("Model path: {}", path);
    paca::fileformats::AssetPack result = modelToPacaFormat(path, size);

    std::ostringstream oss;
    paca::fileformats::Serializer serializer(oss);
    serializer(result);

    if (oss.str().size() == size) return 0;

    std::println("Serialized size: {}", oss.str().size());
    std::println("Computed size: {}", size);
    return -1;
}
