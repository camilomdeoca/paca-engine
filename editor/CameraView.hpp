#pragma once

#include <ResourceFileFormats.hpp>
#include <QWidget>

class CameraView : public QWidget {
public:
    CameraView(paca::fileformats::Scene *scene, QWidget *parent = nullptr);
    virtual ~CameraView();

private:
    paca::fileformats::Scene *m_scene;
};
