#pragma once

#include "Serializer.hpp"
#include "StructEditor.hpp"

#include <ResourceFileFormats.hpp>

#include <QWidget>

class AssetPackEditor : public QWidget
{
public:
    AssetPackEditor(paca::fileformats::AssetPack *assetPack, QWidget *parent = nullptr);
    ~AssetPackEditor();

    void onAddModel();
    void onAddCubemap();

private:
    paca::fileformats::AssetPack *m_assetPack;

    StructEditor<
        paca::fileformats::AssetPack,
        paca::fileformats::Serializer,
        paca::fileformats::Unserializer
    > *m_structEditor;

    FilepathEdit *m_modelFilepathEdit, *m_cubemapFilepathEdit;
};
