#include "CameraView.hpp"
#include "MainWindow.hpp"
#include "AssetPackEditor.hpp"
#include "SceneEditor.hpp"
#include "serializers/BinarySerialization.hpp"

#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QSizePolicy>

#include <QDockWidget>

MainWindow::MainWindow()
    : QMainWindow(nullptr)
{
    setWindowRole("Editor");
    setWindowTitle("Editor");

    resize(1280, 720);
    std::shared_ptr<paca::fileformats::Scene> scene
        = std::make_shared<paca::fileformats::Scene>();
    std::shared_ptr<paca::fileformats::AssetPack> assetPack
        = std::make_shared<paca::fileformats::AssetPack>();

    {
        serialization::BinaryUnserializer unserializer("build/out.pack");
        unserializer << *assetPack;
    }
    {
        serialization::BinaryUnserializer unserializer("build/scene.scene");
        unserializer << *scene;
    }

    //{
    //    QDockWidget *dockWidget = new QDockWidget("Scene Editor");

    //    auto *resourcePackEditor
    //        = new StructEditor<paca::fileformats::Scene>(scene);

    //    dockWidget->setWidget(resourcePackEditor);
    //    addDockWidget(Qt::RightDockWidgetArea, dockWidget);
    //}
    {
        QDockWidget *dockWidget = new QDockWidget("AssetPack Editor");

        auto *assetPackEditor = new AssetPackEditor(assetPack, this);
        dockWidget->setWidget(assetPackEditor);
        addDockWidget(Qt::RightDockWidgetArea, dockWidget);
    }
    {
        QDockWidget *dockWidget = new QDockWidget("Scene Editor");

        SceneEditor *sceneEditor = new SceneEditor(scene, this);
        dockWidget->setWidget(sceneEditor);
        addDockWidget(Qt::RightDockWidgetArea, dockWidget);
    }
    {
        QDockWidget *dockWidget = new QDockWidget("CameraView");
        CameraView *cameraView = new CameraView(scene, assetPack);
        dockWidget->setWidget(cameraView);
        addDockWidget(Qt::LeftDockWidgetArea, dockWidget);
    }
}

MainWindow::~MainWindow()
{}
