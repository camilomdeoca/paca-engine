#pragma once

#include "FilepathEdit.hpp"
#include "StructEditor.hpp"

#include <ResourceFileFormats.hpp>

#include <QWidget>

class AssetPackEditor : public QWidget
{
public:
    AssetPackEditor(const std::shared_ptr<paca::fileformats::AssetPack> &assetPack, QWidget *parent = nullptr);
    ~AssetPackEditor();

    void onImportModels();
    void onImportCubemap();
    void onImportFonts();
    void onOpen();
    void onSave();
    void onSaveAs();

private:
    void createMenuBar(QLayout *layout);

    std::shared_ptr<paca::fileformats::AssetPack> m_assetPack;

    StructEditor<paca::fileformats::AssetPack> *m_structEditor;

    std::string m_assetPackFilepath;
    FilepathEdit *m_modelFilepathEdit, *m_cubemapFilepathEdit, *m_fontFilepathEdit;
    QAction *m_openAction, *m_saveAction, *m_saveAsAction, *m_importAssetAction;
};
