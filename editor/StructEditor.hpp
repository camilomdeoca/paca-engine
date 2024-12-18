#pragma once

#include "StructModel.hpp"

#include <QWidget>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QLabel>
#include <QTreeView>
#include <QMenu>
#include <QPushButton>

template <typename T>
class StructEditor : public QTreeView
{
public:

    StructEditor(QWidget *parent = nullptr)
        : StructEditor(nullptr, {}, parent) {}

    StructEditor(T &refToStruct, QWidget *parent = nullptr)
        : QTreeView(parent), m_refToStruct(refToStruct), m_model(refToStruct)
    {

        setModel(&m_model);
        setItemDelegate(new StructEditorItemDelegate(this)); 

        setContextMenuPolicy(Qt::CustomContextMenu);
        
        createContextMenu();

        QObject::connect(this, &QTreeView::customContextMenuRequested, [this](const QPoint &point) {
            INFO("Contex menu requested");
            QModelIndex index = this->indexAt(point);
            AbstractNode *childItem = static_cast<AbstractNode*>(index.internalPointer());

            if (!childItem) return;

            if (childItem->isInsertable() || (childItem->parentItem() && childItem->parentItem()->isInsertable()))
            {
                m_contextMenuPos = point;
                m_contextMenu->clear();
                if (childItem->isInsertable())
                {
                    m_appendRowMenu->clear();
                    m_contextMenu->addMenu(m_appendRowMenu);
                    const std::vector<std::string_view> &insertOptions = childItem->getInsertOptions();
                    for (size_t i = 0; i < insertOptions.size(); i++)
                    {
                        std::string_view insertOption = insertOptions[i];
                        QAction *action = new QAction(QString::fromUtf8(insertOption), m_appendRowMenu);
                        QObject::connect(action, &QAction::triggered, [this, i]() {
                            QModelIndex index = this->indexAt(m_contextMenuPos);
                            INFO("Append row {} {}", index.row(), index.parent().row());
                            AbstractNode *childItem = static_cast<AbstractNode*>(index.internalPointer());
                            m_model.insertRows(childItem->childCount(), 1, index, i);
                        });
                        m_appendRowMenu->addAction(action);
                    }
                }
                if (childItem->parentItem() && childItem->parentItem()->isInsertable())
                {
                    m_contextMenu->addAction(m_insertRowBeforeAction);
                    m_contextMenu->addAction(m_insertRowAfterAction);
                    m_contextMenu->addAction(m_removeRowAction);
                }
                m_contextMenu->exec(this->viewport()->mapToGlobal(point));
            }
        });
    }

    virtual ~StructEditor() {}

    void rebuildModel()
    {
        m_model.rebuild();
    }

    T &getStruct() { return m_refToStruct; }
    const T &getStruct() const { return m_refToStruct; }

private:

    void createContextMenu()
    {
        m_contextMenu = new QMenu(this);

        m_insertRowBeforeAction = new QAction("Insert row before", this);
        QObject::connect(m_insertRowBeforeAction, &QAction::triggered, [this]() {
            QModelIndex index = this->indexAt(m_contextMenuPos);
            INFO("Insert row before {} {}", index.row(), index.parent().row());
            m_model.insertRows(index.row(), 1, index.parent());
        });

        m_insertRowAfterAction = new QAction("Insert row after", this);
        QObject::connect(m_insertRowAfterAction, &QAction::triggered, [this]() {
            QModelIndex index = this->indexAt(m_contextMenuPos);
            INFO("Insert row after {} {}", index.row(), index.parent().row());
            m_model.insertRows(index.row() + 1, 1, index.parent());
        });

        m_removeRowAction = new QAction("Remove row", this);
        QObject::connect(m_removeRowAction, &QAction::triggered, [this]() {
            QModelIndex index = this->indexAt(m_contextMenuPos);
            INFO("Remove row");
            m_model.removeRows(index.row(), 1, index.parent());
        });

        // For vectors
        m_appendRowMenu = new QMenu("Append row", this);
    }

    T &m_refToStruct;
    StructModel<T> m_model;

    QMenu *m_contextMenu;
    QAction *m_insertRowAfterAction, *m_insertRowBeforeAction, *m_removeRowAction;
    QMenu *m_appendRowMenu;

    QPoint m_contextMenuPos;
};
