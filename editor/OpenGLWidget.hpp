#pragma once

#include "engine/ForwardRenderer.hpp"
#include "engine/PerspectiveCamera.hpp"

#include <QOpenGLWidget>

class OpenGLWidget : public QOpenGLWidget
{
public:
    OpenGLWidget(
        const flecs::world &scene,
        const paca::fileformats::AssetPack &assetPack,
        QWidget *parent = nullptr);

    void reload();

protected:
    void initializeGL() override;

    void resizeGL(int w, int h) override;

    void paintGL() override;

private:
    PerspectiveCamera m_camera;
    engine::ForwardRenderer m_renderer;
    NewResourceManager m_resourceManager;
    const flecs::world &m_scene;
    const paca::fileformats::AssetPack &m_assetPack;
};
