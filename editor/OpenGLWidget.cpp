#include <GL/glew.h>

#include "OpenGLWidget.hpp"

#include <opengl/gl.hpp>

OpenGLWidget::OpenGLWidget(
    const flecs::world &scene,
    const paca::fileformats::AssetPack &assetPack,
    QWidget *parent)
    : QOpenGLWidget(parent),
      m_camera((float)320 / 240, 90.0f),
      m_scene(scene),
      m_assetPack(assetPack)
{}

void OpenGLWidget::reload()
{
    m_resourceManager.loadAssetPack(const_cast<paca::fileformats::AssetPack&>(m_assetPack));
}

void OpenGLWidget::initializeGL()
{
    if (glewInit() != GLEW_OK) {
        ERROR("Error initializing glew.");
        ASSERT(false);
    }

    reload();

    engine::ForwardRenderer::Parameters rendererParams;
    rendererParams.width = 320;
    rendererParams.height = 240;
    rendererParams.flags =
        //ForwardRenderer::Parameters::enableParallaxMapping |
        engine::ForwardRenderer::Parameters::enableShadowMapping;

    GL::init();
    m_renderer.init(rendererParams);

    m_camera.setPosition({0.0f, 0.0f, -10.0f});
}

void OpenGLWidget::resizeGL(int w, int h)
{
    INFO("SIZE: {}, {}", w, h);
    //GL::viewport(w, h);
}

void OpenGLWidget::paintGL()
{
    INFO("PAINT");
    /* Draw */
    GL::setClearColor({0.1f, 0.1f, 0.15f, 1.0f});
    GL::clear();

    m_renderer.renderWorld(
        m_camera,
        m_scene,
        m_resourceManager);
}
