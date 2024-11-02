#pragma once

#include <QWidget>
#include <QLineEdit>

class FilepathEdit : public QWidget
{
    Q_OBJECT

public:
    FilepathEdit(QWidget *parent = nullptr);
    FilepathEdit(const QString &initialPath, const QString &fileExtensionFilter, QWidget *parent = nullptr);
    virtual ~FilepathEdit();

    QString fileName() const { return m_fileNameLineEdit->text(); }

    void setFolderMode(bool state) { m_folderMode = state; }

signals:
    void fileSelected(const QString &fileName);

private slots:
    void onFileSelected();

private:
    bool m_folderMode;
    QLineEdit *m_fileNameLineEdit;
    QString m_fileExtensionFilter;
};
