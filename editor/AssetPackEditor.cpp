#include "AssetPackEditor.hpp"

#include "CombineAssetPack.hpp"
#include "CubemapConverter.hpp"
#include "FontConverter.hpp"
#include "ModelConverter.hpp"
#include <serializers/BinarySerialization.hpp>
#include "StructEditor.hpp"

#include <QMenuBar>
#include <QFileDialog>

#include <functional>
#include <vector>

AssetPackEditor::AssetPackEditor(const std::shared_ptr<paca::fileformats::AssetPack> &assetPack, QWidget *parent)
    : QWidget(parent),
      m_assetPack(assetPack),
      m_structEditor(new StructEditor<paca::fileformats::AssetPack>(assetPack.get(), this))
{
    QVBoxLayout *verticalLayout = new QVBoxLayout(this);

    createMenuBar(verticalLayout);

    verticalLayout->addWidget(m_structEditor);

    setLayout(verticalLayout);
}

AssetPackEditor::~AssetPackEditor()
{}

void AssetPackEditor::createMenuBar(QLayout *layout)
{
    QMenuBar *menuBar = new QMenuBar(this);
    QMenu *fileMenu = menuBar->addMenu("File");
    fileMenu->setToolTipsVisible(true);

    m_openAction = fileMenu->addAction("Open");
    QObject::connect(m_openAction, &QAction::triggered, this, &AssetPackEditor::onOpen);
    m_saveAction = fileMenu->addAction("Save");
    QObject::connect(m_saveAction, &QAction::triggered, this, &AssetPackEditor::onSave);
    m_saveAction->setEnabled(false);
    m_saveAsAction = fileMenu->addAction("Save As");
    QObject::connect(m_saveAsAction, &QAction::triggered, this, &AssetPackEditor::onSaveAs);
    QMenu *importSubmenu = fileMenu->addMenu("Import Assets");
    importSubmenu->setToolTipsVisible(true);
    QAction *m_importModelsAction = importSubmenu->addAction("Models (*.gltf, *.obj, *.dae)");
    QObject::connect(m_importModelsAction, &QAction::triggered, this, &AssetPackEditor::onImportModels);
    QAction *m_importCubemapAction = importSubmenu->addAction("Cubemap");
    m_importCubemapAction->setToolTip(
        "Import Cubemap (folder with 6 images with names: right.jpg,"
        " left.jpg, top.jpg, bottom.jpg, front.jpg, back.jpg)"
    );
    QObject::connect(m_importCubemapAction, &QAction::triggered, this, &AssetPackEditor::onImportCubemap);
    QAction *m_importFontsAction = importSubmenu->addAction("Fonts (*.ttf)");
    QObject::connect(m_importFontsAction, &QAction::triggered, this, &AssetPackEditor::onImportFonts);

    layout->setMenuBar(menuBar);
}

void AssetPackEditor::onImportModels()
{
    QList<QString> fileNames = QFileDialog::getOpenFileNames(
        nullptr,
        "Import Models",
        "",
        "Models (*.gltf *.obj *.dae)");

    std::vector<std::reference_wrapper<paca::fileformats::AssetPack>> assetPacksRefs = { *m_assetPack };

    std::vector<paca::fileformats::AssetPack> assetPacks;
    for (const QString &fileName : fileNames)
    {
        std::string fileNameStdString = fileName.toStdString();
        assetPacks.emplace_back(modelToPacaFormat(fileNameStdString));
        assetPacksRefs.push_back(assetPacks.back());
    }

    *m_assetPack = combine(assetPacksRefs);
    m_structEditor->rebuildModel();
}

void AssetPackEditor::onImportCubemap()
{
    std::string fileName = QFileDialog::getExistingDirectory(
        nullptr,
        "Import Cubemap (folder with 6 images with names: right.jpg, left.jpg, top.jpg, bottom.jpg, front.jpg, back.jpg)",
        "").toStdString();

    paca::fileformats::AssetPack result = cubemapToPacaFormat(fileName, fileName);

    *m_assetPack = combine({ *m_assetPack, result });
    m_structEditor->rebuildModel();
}

void AssetPackEditor::onImportFonts()
{
    QList<QString> fileNames = QFileDialog::getOpenFileNames(
        nullptr,
        "Import Fonts",
        "",
        "Fonts (*.ttf)");

    std::vector<std::reference_wrapper<paca::fileformats::AssetPack>> assetPacksRefs = { *m_assetPack };

    std::vector<paca::fileformats::AssetPack> assetPacks;
    for (const QString &fileName : fileNames)
    {
        std::string fileNameStdString = fileName.toStdString();
        assetPacks.emplace_back(fontToPacaFormat(fileNameStdString, fileNameStdString));
        assetPacksRefs.push_back(assetPacks.back());
    }

    *m_assetPack = combine(assetPacksRefs);
    m_structEditor->rebuildModel();
}

void AssetPackEditor::onOpen()
{
    m_assetPackFilepath = QFileDialog::getOpenFileName(
        nullptr,
        "Open AssetPack file",
        "",
        "AssetPack (*.pack)").toStdString();

    serialization::BinaryUnserializer unserializer(m_assetPackFilepath);
    unserializer << (*m_assetPack);
    m_structEditor->rebuildModel();
    m_saveAction->setEnabled(true);
}

void AssetPackEditor::onSave()
{
    serialization::BinarySerializer serializer(m_assetPackFilepath);

    // m_structEditor->setEnabled(false);
    // Send this to other thread to not freeze gui
    serializer << (*m_assetPack);
}

void AssetPackEditor::onSaveAs()
{
    m_assetPackFilepath = QFileDialog::getOpenFileName(
        nullptr,
        "Save as AssetPack file",
        "",
        "AssetPack (*.pack)").toStdString();

    serialization::BinarySerializer serializer(m_assetPackFilepath);
    serializer << (*m_assetPack);
    m_saveAction->setEnabled(true);
}

