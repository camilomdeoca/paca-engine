#include "AssetPackEditor.hpp"
#include "CubemapConverter.hpp"
#include "ModelConverter.hpp"
#include "StructEditor.hpp"


AssetPackEditor::AssetPackEditor(paca::fileformats::AssetPack *assetPack, QWidget *parent)
    : QWidget(parent),
      m_assetPack(assetPack),
      m_structEditor(new StructEditor<
          paca::fileformats::AssetPack,
          paca::fileformats::Serializer,
          paca::fileformats::Unserializer>(assetPack, {"Asset Packs (*.pack)", "AssetPack"}, this))
{
    QVBoxLayout *verticalLayout = new QVBoxLayout(this);

    // Model adding
    QHBoxLayout *modelFilepathEditLayout = new QHBoxLayout(this);
    QLabel *modelFilepathEditLabel = new QLabel("Model filepath");
    modelFilepathEditLayout->addWidget(modelFilepathEditLabel);
    m_modelFilepathEdit = new FilepathEdit("", "Model (*)");
    modelFilepathEditLayout->addWidget(m_modelFilepathEdit);
    QPushButton *addModelButton = new QPushButton("Add", this);
    modelFilepathEditLayout->addWidget(addModelButton);
    QObject::connect(addModelButton, &QPushButton::released, this, &AssetPackEditor::onAddModel);
    verticalLayout->addLayout(modelFilepathEditLayout);

    // Cubemap adding
    QHBoxLayout *cubemapFilepathEditLayout = new QHBoxLayout(this);
    QLabel *cubemapFilepathEditLabel = new QLabel("Cubemap filepath");
    cubemapFilepathEditLayout->addWidget(cubemapFilepathEditLabel);
    m_cubemapFilepathEdit = new FilepathEdit("", "Cubemap (*)");
    m_cubemapFilepathEdit->setFolderMode(true);
    cubemapFilepathEditLayout->addWidget(m_cubemapFilepathEdit);
    QPushButton *addCubemapButton = new QPushButton("Add", this);
    cubemapFilepathEditLayout->addWidget(addCubemapButton);
    QObject::connect(addCubemapButton, &QPushButton::released, this, &AssetPackEditor::onAddCubemap);
    verticalLayout->addLayout(cubemapFilepathEditLayout);

    verticalLayout->addWidget(m_structEditor);

    setLayout(verticalLayout);
}

AssetPackEditor::~AssetPackEditor()
{}

void AssetPackEditor::onAddModel()
{
    std::string fileName = m_modelFilepathEdit->fileName().toStdString();
    paca::fileformats::AssetPack result
        = modelToPacaFormat(
            fileName,
            fileName.substr(0, fileName.find_last_of("."))
        );
    *m_assetPack = paca::fileformats::combine({result, *m_assetPack});
    m_structEditor->rebuildModel();
}

void AssetPackEditor::onAddCubemap()
{
    std::string fileName = m_cubemapFilepathEdit->fileName().toStdString();
    paca::fileformats::AssetPack result = cubemapToPacaFormat(fileName, fileName);
    *m_assetPack = paca::fileformats::combine({result, *m_assetPack});
    m_structEditor->rebuildModel();
}
