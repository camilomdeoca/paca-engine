#include "SceneEditor.hpp"

#include <serializers/BinarySerialization.hpp>
#include "StructEditor.hpp"

#include <QMenuBar>
#include <QFileDialog>
#include <QString>

SceneEditor::SceneEditor(const std::shared_ptr<paca::fileformats::Scene> &scene, QWidget *parent)
    : QWidget(parent),
      m_scene(scene),
      m_structEditor(new StructEditor<paca::fileformats::Scene>(*scene, this))
{
    QVBoxLayout *verticalLayout = new QVBoxLayout(this);

    createMenuBar(verticalLayout);

    verticalLayout->addWidget(m_structEditor);

    setLayout(verticalLayout);
}

SceneEditor::~SceneEditor()
{}

void SceneEditor::createMenuBar(QLayout *layout)
{
    QMenuBar *menuBar = new QMenuBar(this);
    QMenu *fileMenu = menuBar->addMenu("File");
    fileMenu->setToolTipsVisible(true);

    m_openAction = fileMenu->addAction("Open");
    QObject::connect(m_openAction, &QAction::triggered, this, &SceneEditor::onOpen);
    m_saveAction = fileMenu->addAction("Save");
    QObject::connect(m_saveAction, &QAction::triggered, this, &SceneEditor::onSave);
    m_saveAction->setEnabled(false);
    m_saveAsAction = fileMenu->addAction("Save As");
    QObject::connect(m_saveAsAction, &QAction::triggered, this, &SceneEditor::onSaveAs);

    layout->setMenuBar(menuBar);
}

void SceneEditor::onOpen()
{
    m_sceneFilepath = QFileDialog::getOpenFileName(
        nullptr,
        "Open Scene file",
        "",
        "Scene (*.scene)").toStdString();

    serialization::BinaryUnserializer unserializer(m_sceneFilepath);
    unserializer << (*m_scene);
    m_structEditor->rebuildModel();
    m_saveAction->setEnabled(true);
}

void SceneEditor::onSave()
{
    serialization::BinarySerializer serializer(m_sceneFilepath);

    // m_structEditor->setEnabled(false);
    // Send this to other thread to not freeze gui
    serializer << (*m_scene);
}

void SceneEditor::onSaveAs()
{
    m_sceneFilepath = QFileDialog::getSaveFileName(
        nullptr,
        "Save as Scene file",
        "",
        "Scene (*.scene)").toStdString();

    serialization::BinarySerializer serializer(m_sceneFilepath);
    serializer << (*m_scene);
    m_saveAction->setEnabled(true);
}
