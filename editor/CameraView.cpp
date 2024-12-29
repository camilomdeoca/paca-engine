#include "CameraView.hpp"

#include "OpenGLWidget.hpp"

#include <qpushbutton.h>
#include <utils/Assert.hpp>
#include <utils/Log.hpp>

#include <QtOpenGLWidgets/QOpenGLWidget>
#include <QVBoxLayout>

CameraView::CameraView(
    const std::shared_ptr<paca::fileformats::Scene> &scene,
    const std::shared_ptr<paca::fileformats::AssetPack> &assetPack,
    QWidget *parent)
    : QWidget(parent),
      m_scene(scene)
{
    m_sceneManager.loadScene(*m_scene);
    m_openGLWidget = new OpenGLWidget(m_sceneManager.getFlecsWorld(), *assetPack, this);
    QPushButton *reloadButton = new QPushButton("Reload");
    QObject::connect(reloadButton, &QPushButton::pressed, [this]() {
        m_sceneManager.loadScene(*m_scene);
        m_openGLWidget->reload();
    });
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_openGLWidget);
    layout->addWidget(reloadButton);
    setLayout(layout);
}

CameraView::~CameraView()
{}
