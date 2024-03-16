#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

struct MeshData {
    std::vector<float> vertices;
    std::vector<uint32_t> indices;
};

class MeshLoader {
public:
    static std::optional<MeshData> loadObj(const std::string &path);
};
