#include "CameraView.hpp"

#include <utils/Assert.hpp>
#include <utils/Log.hpp>

#include <GL/glew.h>
#include <opengl/gl.hpp>
#include <QtOpenGLWidgets/QOpenGLWidget>
#include <QVBoxLayout>

class OpenGLWidget : public QOpenGLWidget
{
public:
    OpenGLWidget(QWidget *parent = nullptr)
        : QOpenGLWidget(parent)
    {}

protected:
    void initializeGL() override
    {
        if (glewInit() != GLEW_OK) {
            ERROR("Error initializing glew.");
            ASSERT(false);
        }
        GL::init();
    }

    void resizeGL(int w, int h) override
    {
        INFO("SIZE: {}, {}", w, h);
        GL::viewport(w, h);
    }

    void paintGL() override
    {
        INFO("PAINT");
        GL::clear();
    }

};

CameraView::CameraView(paca::fileformats::Scene *scene, QWidget *parent)
    : QWidget(parent), m_scene(scene)
{
    OpenGLWidget *widget = new OpenGLWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(widget);
    setLayout(layout);
}

CameraView::~CameraView()
{}
