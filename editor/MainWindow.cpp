#include "MainWindow.hpp"
#include "AssetPackEditor.hpp"
#include "CameraView.hpp"
#include "StructEditor.hpp"

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
    paca::fileformats::Scene *scene = new paca::fileformats::Scene();
    {
        QDockWidget *dockWidget = new QDockWidget("Scene Editor");

        auto *resourcePackEditor
            = new StructEditor<paca::fileformats::Scene>(scene);

        dockWidget->setWidget(resourcePackEditor);
        addDockWidget(Qt::RightDockWidgetArea, dockWidget);
    }
    {
        std::shared_ptr<paca::fileformats::AssetPack> resourcePack
            = std::make_shared<paca::fileformats::AssetPack>();
        QDockWidget *dockWidget = new QDockWidget("ResourcePack Editor");

        auto *resourcePackEditor = new AssetPackEditor(resourcePack, this);
        dockWidget->setWidget(resourcePackEditor);
        addDockWidget(Qt::RightDockWidgetArea, dockWidget);
    }
    {
        QDockWidget *dockWidget = new QDockWidget("CameraView");
        CameraView *cameraView = new CameraView(scene);
        dockWidget->setWidget(cameraView);
        addDockWidget(Qt::LeftDockWidgetArea, dockWidget);
    }
}

MainWindow::~MainWindow()
{}
