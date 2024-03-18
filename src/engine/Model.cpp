#include "Model.hpp"
#include "util/ModelLoader.hpp"
#include <cstdio>

Model::Model(const std::string &path)
{
    std::optional<ModelData> data = ModelLoader::loadModel(path);

    if (!data.has_value())
    {
        fprintf(stderr, "Error loading model: %s.\n", path.c_str());
        exit(1);
    }

    m_meshes = data.value().meshes;
    fprintf(stdout, "%s model has %zu meshes.\n", path.c_str(), m_meshes.size());
}

Model::~Model()
{}
