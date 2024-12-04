#pragma once

#include "StructEditor.hpp"

#include <ResourceFileFormats.hpp>

#include <QWidget>

class SceneEditor : public QWidget
{
public:
    SceneEditor(const std::shared_ptr<paca::fileformats::Scene> &scene, QWidget *parent = nullptr);
    ~SceneEditor();

    void onOpen();
    void onSave();
    void onSaveAs();

private:
    void createMenuBar(QLayout *layout);

    std::shared_ptr<paca::fileformats::Scene> m_scene;

    StructEditor<paca::fileformats::Scene> *m_structEditor;

    std::string m_sceneFilepath;
    QAction *m_openAction, *m_saveAction, *m_saveAsAction;
};
