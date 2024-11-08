#pragma once

#include "engine/Light.hpp"

#include <memory>
#include <vector>

class World {
public:
    void addModel(const std::shared_ptr<Model> &model);
    void addModels(const std::vector<std::shared_ptr<Model>> &models);

    void addPointLight(const std::shared_ptr<PointLight> &pointLight);
    void addPointLights(const std::vector<std::shared_ptr<PointLight>> &pointLights);

    void addDirectionalLight(const std::shared_ptr<DirectionalLight> &directionalLight);
    void addDirectionalLights(const std::vector<std::shared_ptr<DirectionalLight>> &directionalLights);

    void setSkybox(const std::shared_ptr<Texture> &skybox);

    void update(float deltaTime);

    const std::vector<std::shared_ptr<Model>> &getModels() const { return m_models; }
    const std::vector<std::shared_ptr<PointLight>> &getPointLights() const { return m_pointLights; }
    const std::vector<std::shared_ptr<DirectionalLight>> &getDirectionalLights() const { return m_directionalLights; }
    const std::shared_ptr<Texture> &getSkyboxTexture() const { return m_skybox; }

private:
    std::vector<std::shared_ptr<Model>> m_models;
    std::vector<std::shared_ptr<PointLight>> m_pointLights;
    std::vector<std::shared_ptr<DirectionalLight>> m_directionalLights;
    std::shared_ptr<Texture> m_skybox;
};
