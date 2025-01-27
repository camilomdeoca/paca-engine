#include "engine/FlecsSerialization.hpp"

#include "engine/Components.hpp"

namespace engine::serializers {

void FlecsSerializer::operator()(flecs::world &world)
{
    m_serializer.getYamlEmitter() << YAML::BeginMap;
    m_serializer.getYamlEmitter() << YAML::Key << "sceneEntities";
    m_serializer.getYamlEmitter() << YAML::Value << YAML::BeginSeq;
    world.each([this](
        flecs::entity e,
        engine::components::Transform *c1,
        engine::components::Material *c2,
        engine::components::StaticMesh *c3,
        engine::components::AnimatedMesh *c4,
        engine::components::AnimationPlayer *c5,
        engine::components::PointLight *c6,
        engine::components::DirectionalLight *c7,
        engine::components::Skybox *c8,
        engine::components::Camera *c9,
        engine::tags::SceneEntityTag
    ) {
        m_serializer.getYamlEmitter() << YAML::BeginMap;
        m_serializer.getYamlEmitter() << YAML::Key << "id" << YAML::Value << e.id();
        m_serializer.getYamlEmitter() << YAML::Key << "name" << YAML::Value << e.name();

        m_serializer.getYamlEmitter() << YAML::Key << "components";
        m_serializer.getYamlEmitter() << YAML::Value << YAML::BeginSeq;
        if (c1) m_serializer(c1->getClassName(), *c1);
        if (c2) m_serializer(c2->getClassName(), *c2);
        if (c3) m_serializer(c3->getClassName(), *c3);
        if (c4) m_serializer(c4->getClassName(), *c4);
        if (c5) m_serializer(c5->getClassName(), *c5);
        if (c6) m_serializer(c6->getClassName(), *c6);
        if (c7) m_serializer(c7->getClassName(), *c7);
        if (c8) m_serializer(c8->getClassName(), *c8);
        if (c8) m_serializer(c8->getClassName(), *c9);
        m_serializer.getYamlEmitter() << YAML::EndSeq;
        m_serializer.getYamlEmitter() << YAML::EndMap;
    });


    m_serializer.getYamlEmitter() << YAML::EndSeq;
    m_serializer.getYamlEmitter() << YAML::Key << "sceneComponents";
    m_serializer.getYamlEmitter() << YAML::Value << YAML::BeginSeq;
    if (auto c = world.get<engine::components::Transform>()) m_serializer(c->getClassName(), *c);
    if (auto c = world.get<engine::components::Material>()) m_serializer(c->getClassName(), *c);
    if (auto c = world.get<engine::components::StaticMesh>()) m_serializer(c->getClassName(), *c);
    if (auto c = world.get<engine::components::AnimatedMesh>()) m_serializer(c->getClassName(), *c);
    if (auto c = world.get<engine::components::AnimationPlayer>()) m_serializer(c->getClassName(), *c);
    if (auto c = world.get<engine::components::PointLight>()) m_serializer(c->getClassName(), *c);
    if (auto c = world.get<engine::components::DirectionalLight>()) m_serializer(c->getClassName(), *c);
    if (auto c = world.get<engine::components::Skybox>()) m_serializer(c->getClassName(), *c);
    m_serializer.getYamlEmitter() << YAML::EndSeq;
    m_serializer.getYamlEmitter() << YAML::EndMap;
}

void FlecsUnserializer::operator()(flecs::world &world)
{
    world.reset();

    world.component<engine::tags::SceneEntityTag>("SceneEntityTag");

    world.component<engine::components::Transform>("Transform");
    world.component<engine::components::Material>("Material");
    world.component<engine::components::StaticMesh>("StaticMesh");
    world.component<engine::components::AnimatedMesh>("AnimatedMesh");
    world.component<engine::components::AnimationPlayer>("Animation");
    world.component<engine::components::PointLight>("PointLight");
    world.component<engine::components::DirectionalLight>("DirectionalLight");
    world.component<engine::components::DirectionalLightShadowMap>("DirectionalLightShadowMap");
    world.component<engine::components::Skybox>("Skybox");
    world.component<engine::components::Camera>("Camera");

    YAML::Node rootNode = m_unserializer.getYamlNode();
    YAML::Node sceneEntities = rootNode["entities"];
    for (size_t i = 0; i < sceneEntities.size(); i++)
    {
        YAML::Node entityNode = sceneEntities[i];
        flecs::entity e = world.entity()
            .set_name(entityNode["name"].as<std::string>("UNNAMED").c_str())
            .add<engine::tags::SceneEntityTag>();

        YAML::Node entityComponents = entityNode["components"];
        for (size_t i = 0; i < entityComponents.size(); i++)
        {
            if (!entityComponents[i].IsMap())
            {
                ERROR("Invalid component in entity");
                continue;
            }

            std::string typeName = entityComponents[i].begin()->first.as<std::string>();
            YAML::Node componentNode = entityComponents[i][typeName];
            if (typeName == "Transform") m_unserializer(e.ensure<engine::components::Transform>(), componentNode);
            if (typeName == "Material") m_unserializer(e.ensure<engine::components::Material>(), componentNode);
            if (typeName == "StaticMesh") m_unserializer(e.ensure<engine::components::StaticMesh>(), componentNode);
            if (typeName == "AnimatedMesh") m_unserializer(e.ensure<engine::components::AnimatedMesh>(), componentNode);
            if (typeName == "Animation") m_unserializer(e.ensure<engine::components::AnimationPlayer>(), componentNode);
            if (typeName == "PointLight") m_unserializer(e.ensure<engine::components::PointLight>(), componentNode);
            if (typeName == "DirectionalLight") {
                m_unserializer(e.ensure<engine::components::DirectionalLight>(), componentNode);
                e.template emplace<engine::components::DirectionalLightShadowMap>(
                    512u,
                    std::vector<float>{2.5f, 5.0f, 10.0f, 20.0f, 50.0f});
            }
            if (typeName == "Skybox") m_unserializer(e.ensure<engine::components::Skybox>(), componentNode);
            if (typeName == "Camera") m_unserializer(e.ensure<engine::components::Camera>(), componentNode);
        }
    }

    YAML::Node entityComponents = rootNode["sceneComponents"];
    for (size_t i = 0; i < entityComponents.size(); i++)
    {
        std::string typeName = entityComponents[i].begin()->first.as<std::string>();
        YAML::Node componentNode = entityComponents[i][typeName];
        if (typeName == "Transform") m_unserializer(world.ensure<engine::components::Transform>(), componentNode);
        if (typeName == "Material") m_unserializer(world.ensure<engine::components::Material>(), componentNode);
        if (typeName == "StaticMesh") m_unserializer(world.ensure<engine::components::StaticMesh>(), componentNode);
        if (typeName == "AnimatedMesh") m_unserializer(world.ensure<engine::components::AnimatedMesh>(), componentNode);
        if (typeName == "Animation") m_unserializer(world.ensure<engine::components::AnimationPlayer>(), componentNode);
        if (typeName == "PointLight") m_unserializer(world.ensure<engine::components::PointLight>(), componentNode);
        if (typeName == "DirectionalLight") m_unserializer(world.ensure<engine::components::DirectionalLight>(), componentNode);
        if (typeName == "Skybox") m_unserializer(world.ensure<engine::components::Skybox>(), componentNode);
        if (typeName == "Camera") m_unserializer(world.ensure<engine::components::Camera>(), componentNode);
    }
}

} // namespace engine::serializers
