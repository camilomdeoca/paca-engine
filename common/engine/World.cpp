#include "engine/World.hpp"

#include "engine/Light.hpp"

#include <memory>
#include <vector>

void World::addModel(const std::shared_ptr<Model> &model)
{
    m_models.emplace_back(model);
}

void World::addModels(const std::vector<std::shared_ptr<Model>> &models)
{
    for (const std::shared_ptr<Model> &model : models)
    {
        addModel(model);
    }
}

void World::addPointLight(const std::shared_ptr<PointLight> &pointLight)
{
    m_pointLights.emplace_back(pointLight);
}

void World::addPointLights(const std::vector<std::shared_ptr<PointLight>> &pointLights)
{
    for (const std::shared_ptr<PointLight> &pointLight : pointLights)
    {
        addPointLight(pointLight);
    }
}

void World::addDirectionalLight(const std::shared_ptr<DirectionalLight> &directionalLight)
{
    m_directionalLights.emplace_back(directionalLight);
}

void World::addDirectionalLights(const std::vector<std::shared_ptr<DirectionalLight>> &directionalLights)
{
    for (const std::shared_ptr<DirectionalLight> &directionalLight : directionalLights)
    {
        addDirectionalLight(directionalLight);
    }
}

void World::setSkybox(const std::shared_ptr<Texture> &skybox)
{
    m_skybox = skybox;
}
