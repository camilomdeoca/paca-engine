#include "engine/Model.hpp"

Model::Model(const std::vector<std::shared_ptr<Mesh>> &meshes)
    : m_meshes(meshes)
{}

Model::~Model()
{}
