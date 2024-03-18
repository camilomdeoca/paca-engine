#pragma once

#include "engine/Mesh.hpp"

#include <memory>
#include <optional>
#include <vector>

struct ModelData {
    std::vector<std::shared_ptr<Mesh>> meshes;
};

class ModelLoader {
public:
    static std::optional<ModelData> loadModel(const std::string &path);
};
