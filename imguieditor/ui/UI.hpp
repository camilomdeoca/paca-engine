#pragma once

#include "../AssetMetadataManager.hpp"
#include <engine/ForwardRenderer.hpp>
#include <engine/Components.hpp>

#include <ResourceFileFormats.hpp>

#include <flecs.h>

class PerspectiveCamera;
class AssetManager;
class FrameBuffer;

class UI
{
public:
    struct Context
    {
        AssetManager &assetManager;
        AssetMetadataManager &assetMetadataManager;
        engine::ForwardRenderer &renderer;
        flecs::world &world;
    };

    UI(const Context &context);

    void draw();

    void update(float timeDelta) { m_time += timeDelta; m_timeDelta = timeDelta; }

private:
    void drawStats();
    void drawSceneTree();
    void drawSceneView();
    void drawAssetManager();
    void drawStaticMeshBrowser();
    void drawAnimatedMeshBrowser();
    void drawMaterialBrowser();
    void drawSelectedEntity();
    void drawCameraParameters();
    void drawShadowMapsDebug();
    void drawCameraEntityViews();

    void addStaticMesh(const char *path);

    void componentEdit(engine::components::Transform &transformComponent);
    void componentEdit(engine::components::Material &materialComponent);
    void componentEdit(engine::components::StaticMesh &staticMeshComponent);
    void componentEdit(engine::components::AnimatedMesh &animatedMeshComponent);
    void componentEdit(engine::components::AnimationPlayer &animationPlayerComponent);
    void componentEdit(engine::components::PointLight &pointLightComponent);
    void componentEdit(engine::components::DirectionalLight &directionalLightComponent);
    void componentEdit(engine::components::Skybox &skyboxComponent);
    void componentEdit(engine::components::Camera &cameraComponent);

    AssetManager &m_assetManager;
    AssetMetadataManager &m_assetMetadataManager;
    engine::ForwardRenderer &m_renderer;
    FrameBuffer m_renderTarget;
    flecs::world &m_world;

    engine::components::Transform m_cameraTransform;
    engine::components::Camera m_camera;

    struct CameraEntityView
    {
        FrameBuffer framebuffer;
        flecs::entity entity;
    };

    std::vector<CameraEntityView> m_cameraViews;

    flecs::entity m_selectedEntity;
    float m_time;
    float m_timeDelta;
};
