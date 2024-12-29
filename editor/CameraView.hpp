#pragma once

#include <engine/SceneManager.hpp>

#include <QWidget>

class OpenGLWidget;

namespace paca::fileformats {
    struct Scene;
    struct AssetPack;
}

class CameraView : public QWidget {
public:
    CameraView(
        const std::shared_ptr<paca::fileformats::Scene> &scene,
        const std::shared_ptr<paca::fileformats::AssetPack> &assetPack,
        QWidget *parent = nullptr);

    virtual ~CameraView();

private:
    OpenGLWidget *m_openGLWidget;
    std::shared_ptr<paca::fileformats::Scene> m_scene;
    engine::SceneManager m_sceneManager;
};
