#pragma once

#include "engine/Mesh.hpp"

#include <memory>
#include <string>
#include <vector>

class Model {
public:
    Model(const std::string &path);
    ~Model();

    std::vector<std::shared_ptr<Mesh>> &getMeshes() { return m_meshes; }

private:
    std::vector<std::shared_ptr<Mesh>> m_meshes;
};
