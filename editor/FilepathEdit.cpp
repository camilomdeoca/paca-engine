#include "FilepathEdit.hpp"
#include "utils/Log.hpp"

#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QFileDialog>

FilepathEdit::FilepathEdit(QWidget *parent)
    : FilepathEdit("", "", parent)
{}

FilepathEdit::FilepathEdit(const QString &initialPath, const QString &fileExtensionFilter, QWidget *parent)
    : QWidget(parent), m_folderMode(false), m_fileExtensionFilter(fileExtensionFilter)
{
    QHBoxLayout *horizontalLayout = new QHBoxLayout(this);
    m_fileNameLineEdit = new QLineEdit(initialPath);
    m_fileNameLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    horizontalLayout->addWidget(m_fileNameLineEdit);
    QPushButton *browseFilesButton = new QPushButton("...", this);

    QObject::connect(browseFilesButton, &QPushButton::released, this, &FilepathEdit::onFileSelected);

    browseFilesButton->setMaximumWidth(40);
    horizontalLayout->addWidget(browseFilesButton);
    setLayout(horizontalLayout);
}

FilepathEdit::~FilepathEdit()
{}

void FilepathEdit::onFileSelected()
{
    INFO("CLICK");
    QString text;
    if (m_folderMode)
        text = QFileDialog::getExistingDirectory(nullptr, "Open Folder", "");
    else
        text = QFileDialog::getOpenFileName(nullptr, "Open File", "", m_fileExtensionFilter);
    m_fileNameLineEdit->setText(text);
    emit fileSelected(m_fileNameLineEdit->text());
}

