#include "engine/SceneManager.hpp"

#include <ResourceFileFormats.hpp>
#include <engine/components/Components.hpp>
#include <utils/Log.hpp>

#include <variant>

namespace engine {

template <typename EntityType>
struct ComponentVisitor {
    ComponentVisitor(const EntityType &entity)
        : m_entity(entity)
    {}

    void operator()(const paca::fileformats::components::Transform &transform)
    {
        INFO("\t {}", transform.getClassName());
        m_entity.template set<engine::components::Transform>({
            transform.position,
            transform.rotation,
            transform.scale});
    }

    void operator()(const paca::fileformats::components::Material &material)
    {
        INFO("\t {}", material.getClassName());
        m_entity.template set<engine::components::Material>({material.id});
    }

    void operator()(const paca::fileformats::components::StaticMesh &staticMesh)
    {
        INFO("\t {}", staticMesh.getClassName());
        m_entity.template set<engine::components::StaticMesh>({staticMesh.id});
    }

    void operator()(const paca::fileformats::components::AnimatedMesh &animatedMesh)
    {
        INFO("\t {}", animatedMesh.getClassName());
        m_entity.template set<engine::components::AnimatedMesh>({animatedMesh.id});
    }

    void operator()(const paca::fileformats::components::PointLight &pointLight)
    {
        INFO("\t {}", pointLight.getClassName());
        m_entity.template set<engine::components::PointLight>({
            pointLight.color,
            pointLight.intensity,
            pointLight.attenuation});
    }

    void operator()(const paca::fileformats::components::DirectionalLight &directionalLight)
    {
        INFO("\t {}", directionalLight.getClassName());
        // Cant chain set and emplace calls because if EntityType is flecs::world, then
        // flecs::world::set and flecs::world::emplace do not return the world instance
        m_entity
            .template set<engine::components::DirectionalLight>({
                directionalLight.color,
                directionalLight.intensity});
        m_entity
            .template emplace<engine::components::DirectionalLightShadowMap>(
                512u,
                std::vector<float>{5.0f, 10.0f, 25.0f, 50.0f, 100.0f});
    }

    void operator()(const paca::fileformats::components::Skybox &skybox)
    {
        INFO("\t {}", skybox.getClassName());
        m_entity.template set<engine::components::Skybox>({skybox.id});
    }

private:
    const EntityType &m_entity;
};

void SceneManager::loadScene(const paca::fileformats::Scene &scene)
{
    m_world.reset();
    
    m_world.component<engine::tags::SceneEntityTag>("SceneEntityTag");

    m_world.component<engine::components::Transform>("Transform");
    m_world.component<engine::components::Material>("Material");
    m_world.component<engine::components::StaticMesh>("StaticMesh");
    m_world.component<engine::components::AnimatedMesh>("AnimatedMesh");
    m_world.component<engine::components::PointLight>("PointLight");
    m_world.component<engine::components::DirectionalLight>("DirectionalLight");
    m_world.component<engine::components::DirectionalLightShadowMap>("DirectionalLightShadowMap");
    m_world.component<engine::components::Skybox>("Skybox");

    for (const paca::fileformats::Entity &deserializedEntity : scene.entities)
    {
        INFO("ENTITY");
        flecs::entity entity = m_world.entity()
            .set_name(std::to_string(deserializedEntity.id).c_str())
            .add<tags::SceneEntityTag>();

        for (const paca::fileformats::Component &component : deserializedEntity.components)
        {
            std::visit(ComponentVisitor<flecs::entity>(entity), component);
        }
    }

    for (const paca::fileformats::Component &component : scene.sceneComponents)
    {
        std::visit(ComponentVisitor<flecs::world>(m_world), component);
    }
}

} // namespace engine
