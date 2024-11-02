#pragma once

#include "StructModel.hpp"
#include "FilepathEdit.hpp"

#include <QWidget>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QLabel>
#include <QTreeView>
#include <QMenu>
#include <QPushButton>

#include <fstream>

struct StructEditorParams {
    QString fileExtensionFilter = "";
    QString structName = "Struct";
};

template <typename T, class Serializer, class Unserializer>
requires requires (T t, Serializer serializer, Unserializer unserializer, std::ifstream ifs, std::ofstream ofs)
{
    { Serializer(ofs) } -> std::same_as<Serializer>;
    { Unserializer(ifs) } -> std::same_as<Unserializer>;
}
class StructEditor : public QWidget
{
public:

    StructEditor(QWidget *parent = nullptr)
        : StructEditor(nullptr, {}, parent) {}

    StructEditor(T *ptrToStruct, StructEditorParams &&params = {}, QWidget *parent = nullptr)
        : QWidget(parent), m_ptrToStruct(ptrToStruct), m_model(ptrToStruct)
    {
        QVBoxLayout *verticalLayout = new QVBoxLayout(this);

        // File input
        QHBoxLayout *filepathEditLabelLayout = new QHBoxLayout(this);
        QLabel *filepathEditLabel = new QLabel(params.structName + " filepath");
        filepathEditLabelLayout->addWidget(filepathEditLabel);
        m_filepathEdit = new FilepathEdit("", params.fileExtensionFilter);
        filepathEditLabelLayout->addWidget(m_filepathEdit);
        verticalLayout->addLayout(filepathEditLabelLayout);

        // Tree view
        m_treeView = new QTreeView();
        m_treeView->setModel(&m_model);
        m_treeView->setItemDelegate(new StructEditorItemDelegate(m_treeView)); 
        verticalLayout->addWidget(m_treeView);

        // Load/Save buttons
        QHBoxLayout *loadSaveButtonsLayout = new QHBoxLayout(this);
        QPushButton *loadButton = new QPushButton("Load", this);
        QPushButton *saveButton = new QPushButton("Save", this);
        QObject::connect(loadButton, &QPushButton::released, this, &StructEditor::onLoadFileButtonPress);
        QObject::connect(saveButton, &QPushButton::released, this, &StructEditor::onSaveFileButtonPress);
        loadSaveButtonsLayout->addWidget(loadButton);
        loadSaveButtonsLayout->addWidget(saveButton);
        verticalLayout->addLayout(loadSaveButtonsLayout);

        m_treeView->setContextMenuPolicy(Qt::CustomContextMenu);
        
        createContextMenu();

        QObject::connect(m_treeView, &QTreeView::customContextMenuRequested, [this](const QPoint &point) {
            INFO("Contex menu requested");
            QModelIndex index = m_treeView->indexAt(point);
            AbstractNode *childItem = static_cast<AbstractNode*>(index.internalPointer());

            if (!childItem) return;

            if (childItem->isInsertable() || (childItem->parentItem() && childItem->parentItem()->isInsertable()))
            {
                m_contextMenuPos = point;
                m_contextMenu->clear();
                if (childItem->isInsertable())
                {
                    m_contextMenu->addAction(m_prependRowAction);
                    m_contextMenu->addAction(m_appendRowAction);
                }
                if (childItem->parentItem() && childItem->parentItem()->isInsertable())
                {
                    m_contextMenu->addAction(m_insertRowBeforeAction);
                    m_contextMenu->addAction(m_insertRowAfterAction);
                    m_contextMenu->addAction(m_removeRowAction);
                }
                m_contextMenu->exec(m_treeView->viewport()->mapToGlobal(point));
            }
        });
    
        //QHBoxLayout *horizontalLayout = new QHBoxLayout();
        //verticalLayout->addLayout(horizontalLayout);
    
        setLayout(verticalLayout);
    }

    virtual ~StructEditor() {}

    void rebuildModel()
    {
        m_model.rebuild();
    }

    void onLoadFileButtonPress()
    {
        std::ifstream ifs(m_filepathEdit->fileName().toStdString());
        Unserializer unserializer(ifs);
        unserializer(*m_ptrToStruct);
        m_model.rebuild();
    }

    void onSaveFileButtonPress()
    {
        std::ofstream ofs(m_filepathEdit->fileName().toStdString());
        Serializer serializer(ofs);
        serializer(*m_ptrToStruct);
    }

private:

    void createContextMenu()
    {
        m_contextMenu = new QMenu(this);

        m_insertRowBeforeAction = new QAction("Insert row before", this);
        QObject::connect(m_insertRowBeforeAction, &QAction::triggered, [this]() {
            QModelIndex index = m_treeView->indexAt(m_contextMenuPos);
            INFO("Insert row before {} {}", index.row(), index.parent().row());
            m_model.insertRows(index.row(), 1, index.parent());
        });

        m_insertRowAfterAction = new QAction("Insert row after", this);
        QObject::connect(m_insertRowAfterAction, &QAction::triggered, [this]() {
            QModelIndex index = m_treeView->indexAt(m_contextMenuPos);
            INFO("Insert row after {} {}", index.row(), index.parent().row());
            m_model.insertRows(index.row() + 1, 1, index.parent());
        });

        m_removeRowAction = new QAction("Remove row", this);
        QObject::connect(m_removeRowAction, &QAction::triggered, [this]() {
            QModelIndex index = m_treeView->indexAt(m_contextMenuPos);
            INFO("Remove row");
            m_model.removeRows(index.row(), 1, index.parent());
        });

        // For vectors
        m_prependRowAction = new QAction("Prepend row", this);
        QObject::connect(m_prependRowAction, &QAction::triggered, [this]() {
            QModelIndex index = m_treeView->indexAt(m_contextMenuPos);
            INFO("Prepend row {} {}", index.row(), index.parent().row());
            m_model.insertRows(0, 1, index);
        });

        m_appendRowAction = new QAction("Append row", this);
        QObject::connect(m_appendRowAction, &QAction::triggered, [this]() {
            QModelIndex index = m_treeView->indexAt(m_contextMenuPos);
            INFO("Append row {} {}", index.row(), index.parent().row());
            AbstractNode *childItem = static_cast<AbstractNode*>(index.internalPointer());
            m_model.insertRows(childItem->childCount(), 1, index);
        });
    }

    T *m_ptrToStruct;
    StructModel<T> m_model;

    FilepathEdit *m_filepathEdit;

    QTreeView *m_treeView;

    QMenu *m_contextMenu;
    QAction *m_insertRowAfterAction, *m_insertRowBeforeAction, *m_removeRowAction;
    QAction *m_prependRowAction, *m_appendRowAction;

    QPoint m_contextMenuPos;
};
