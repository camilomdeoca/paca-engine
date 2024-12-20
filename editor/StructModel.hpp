#pragma once

#include "QtUtils.hpp"
#include "utils/Log.hpp"
#include <cstring>
#include <limits>
#include <memory>
#include <type_traits>
#include <utility>
#include <utils/Assert.hpp>

#include <QAbstractItemModel>
#include <QStyledItemDelegate>
#include <QLineEdit>
#include <QIntValidator>
#include <QDoubleValidator>
#include <QDoubleSpinBox>

#include <ResourceFileFormats.hpp>
#include <variant>

template <typename T>
concept glm_type = requires(T t) {
    { t.length() } -> std::same_as<int>;
    { t[0] } -> std::same_as<decltype(t[0])&>;
};

template <typename T>
concept aggregate = std::is_aggregate_v<T>;

namespace detail {

//! @internal
//  @brief Iterate over variant types
template<
    typename VariantType,
    size_t Index,
    std::enable_if_t<Index >= std::variant_size_v<VariantType>, bool> = true>
constexpr void getVariantTypesNamesImpl(std::vector<std::string_view> &names)
{}

//! @internal
//  @brief Iterate over variant types
template<
    typename VariantType,
    size_t Index,
    std::enable_if_t<Index < std::variant_size_v<VariantType>, bool> = true>
constexpr void getVariantTypesNamesImpl(std::vector<std::string_view> &names)
{
    names.emplace_back(std::variant_alternative_t<Index, VariantType>::getClassName());
    getVariantTypesNamesImpl<VariantType, Index+1>(names);
}

template<typename VariantType>
constexpr std::vector<std::string_view> getVariantTypesNames()
{
    std::vector<std::string_view> names;
    getVariantTypesNamesImpl<VariantType, 0>(names);
    return names;
}

template<typename T>
struct is_variant : std::false_type
{};

template<typename ...Args>
struct is_variant<std::variant<Args...>> : std::true_type
{};

template<typename T>
inline constexpr bool is_variant_v=is_variant<T>::value;

}

class AbstractNode
{
public:
    enum class Type : uint32_t {
        None,
        String,
        Int,
        Float,
    };

    AbstractNode(AbstractNode *parent = nullptr)
        : m_parentItem(parent) {}

    virtual ~AbstractNode() {}

    void appendChild(std::unique_ptr<AbstractNode> &&child)
    {
        m_children.push_back(std::move(child));
    }

    AbstractNode *child(int row)
    {
        return row >= 0 && row < childCount() ? m_children[row].get() : nullptr;
    }

    int childCount() const { return m_children.size(); }
    int columnCount() const { return 2; }

    virtual QString fieldName(size_t index) const = 0;

    virtual QVariant data(int column) const = 0;

    int row() const
    {
        if (m_parentItem == nullptr) return 0;
        const auto it = std::find_if(
                m_parentItem->m_children.cbegin(),
                m_parentItem->m_children.cend(),
                [this](const std::unique_ptr<AbstractNode> &treeItem) {
                    return treeItem.get() == this;
                });

        if (it != m_parentItem->m_children.cend())
            return std::distance(m_parentItem->m_children.cbegin(), it);

        ASSERT(false);
        return -1;
    }

    AbstractNode *parentItem() const { return m_parentItem; }

    virtual bool isEditable() const { return false; }
    virtual bool isInsertable() const { return false; }
    virtual const std::vector<std::string_view> &getInsertOptions() const
    {
        ASSERT(false);
        static std::vector<std::string_view> a;
        return a;
    }

    virtual Type type() const { return Type::None; }

    virtual bool setData(int column, const QVariant &value) { ASSERT(false); return false; }
    virtual bool insertChildren(int position, int count, size_t insertOptionIndex = 0) { ASSERT(false); return false; }
    virtual bool removeChildren(int position, int count) { ASSERT(false); return false; }

protected:
    AbstractNode *m_parentItem;
    std::vector<std::unique_ptr<AbstractNode>> m_children;
};

template <typename>
class Node;

namespace detail {

//! @internal
//  @brief If variant add node of base type else add node of type
template<typename... Types>
void addNodeOfTo(std::variant<Types...> &variant, AbstractNode &node)
{
    std::visit([&node](auto &arg) {
        node.appendChild(std::make_unique<Node<std::remove_reference_t<decltype(arg)>>>(&arg, &node));
    }, variant);
}

//! @internal
//  @brief If variant add node of base type else add node of type
template<typename T>
void addNodeOfTo(T &field, AbstractNode &node)
{
    node.appendChild(std::make_unique<Node<std::remove_reference_t<decltype(field)>>>(&field, &node));
}

//! @internal
template<
    size_t N,
    typename... Types,
    std::enable_if_t<N >= std::variant_size_v<std::variant<Types...>>, bool> = true>
void setTypeOnVariant(
    std::variant<Types...> &variant,
    size_t targetTypeIndex)
{
    FATAL("Invalid type index for variant");
}

//! @internal
template<
    size_t N,
    typename... Types,
    std::enable_if_t<N < std::variant_size_v<std::variant<Types...>>, bool> = true> 
void setTypeOnVariant(
    std::variant<Types...> &variant,
    size_t targetTypeIndex)
{
    if (N == targetTypeIndex)
    {
        variant.template emplace<N>();
    }
    else
    {
        setTypeOnVariant<N+1>(variant, targetTypeIndex);
    }
}

}

// Specialization for builtin types (int, float, bool, etc)
template <typename T>
requires std::is_arithmetic_v<T> || std::is_enum_v<T>
class Node <T> : public AbstractNode
{
public:
    Node(T *data, AbstractNode *parent = nullptr)
        : AbstractNode(parent), m_data(data) {}

    virtual QString fieldName(size_t index) const override
    {
        ASSERT(false);
        return {};
    }

    virtual QVariant data(int column) const override
    {
        if (column == 0)
        {
            return parentItem() ? parentItem()->fieldName(row()) : "ROOT";
        }
        if (m_data)
            return qt_utils::toQVariant(*m_data);
        return {};
    }

    virtual bool isEditable() const override { return true; }

    virtual Type type() const override
    {
        if constexpr (std::is_integral_v<T>)
            return Type::Int;
        else if constexpr (std::is_floating_point_v<T>)
            return Type::Float;
        else if constexpr (std::is_enum_v<T>)
            return Type::Int;
        return Type::None;
    }

    virtual bool setData(int column, const QVariant &value) override
    {
        if (column != 1) return false;

        if constexpr (std::is_integral_v<T>)
        {
            if (value.userType() != QMetaType::Int)
            {
                WARN("Type is {}, instead of int", value.typeName());
                return false;
            }
            *m_data = value.toInt();
        }
        else if constexpr (std::is_floating_point_v<T>)
        {
            if (value.userType() != QMetaType::Float && value.userType() != QMetaType::Double)
            {
                WARN("Type is {}, instead of float/double", value.typeName());
                return false;
            }
            *m_data = value.toDouble();
        }
        else if constexpr (std::is_enum_v<T>)
        {
            INFO("Enum is {}", value.typeName());
            *m_data = T(value.toInt());
        }

        return true;
    }

private:
    T *m_data;
};

namespace detail {

template<typename T>
struct Visitor
{
    Visitor(Node<T> &node)
        : m_node(node)
    {}

    Visitor &operator<<(auto &field)
    {
        detail::addNodeOfTo(field, m_node);
        return *this;
    }

    template<typename... FieldTypes>
    void operator()(FieldTypes& ...fields)
    {
        ((*this) << ... << fields);
    }

    Node<T> &m_node;
};

}

// Specialization for structs
template <aggregate T>
class Node <T> : public AbstractNode
{
public:
    Node(T *data, AbstractNode *parent = nullptr)
        : AbstractNode(parent), m_data(data)
    {
        detail::Visitor visitor(*this);
        data->forEachField(visitor);
    }

    virtual QString fieldName(size_t index) const override
    {
        std::string_view name = m_data->getFieldNames()[index];
        return QString::fromUtf8(name.data(), name.size());
    }

    virtual QVariant data(int column) const override
    {
        if (column == 0)
        {
            return parentItem() ? parentItem()->fieldName(row()) : "ROOT";
        }
        if (m_data)
            return qt_utils::toQVariant(*m_data);
        return {};
    }

private:
    T *m_data;
};

// Specialization for glm::vec3 glm::quat glm::mat4 and glm::mat4::row_type (glm::vec4)
template <glm_type T>
class Node <T> : public AbstractNode
{
public:
    Node(T *data, AbstractNode *parent = nullptr)
        : AbstractNode(parent), m_data(data)
    {
        for (int i = 0; i < m_data->length(); i++)
        {
            detail::addNodeOfTo((*m_data)[i], static_cast<AbstractNode&>(*this));
        }
    }

    virtual QString fieldName(size_t index) const override
    {
        return std::array{"x", "y", "z", "w"}[index];
    }

    virtual QVariant data(int column) const override
    {
        if (column == 0)
        {
            return parentItem() ? parentItem()->fieldName(row()) : "ROOT";
        }
        if (m_data)
            return qt_utils::toQVariant(*m_data);
        return {};
    }

private:
    T *m_data;
};

// Specialization for strings
template <>
class Node <std::string> : public AbstractNode
{
public:
    Node(std::string *data, AbstractNode *parent = nullptr)
        : AbstractNode(parent), m_data(data) {}

    virtual QString fieldName(size_t index) const override
    {
        ASSERT(false);
        return {};
    }

    virtual QVariant data(int column) const override
    {
        if (column == 0)
        {
            return parentItem() ? parentItem()->fieldName(row()) : "ROOT";
        }
        if (m_data)
            return qt_utils::toQVariant(*m_data);
        return {};
    }

    virtual bool isEditable() const override { return true; }

    virtual Type type() const override { return Type::String; }

    virtual bool setData(int column, const QVariant &value) override
    {
        if (column != 1) return false;
        
        if (value.userType() != QMetaType::QString)
        {
            WARN("Value is {}, instead of string", value.typeName());
            return false;
        }

        *m_data = value.toString().toStdString();

        return true;
    }

private:
    std::string *m_data;
};

// Specialization for arrays of any type
template <class ElementType, typename SizeType, SizeType size>
class Node <std::array<ElementType, size>> : public AbstractNode
{
public:
    Node(std::array<ElementType, size> *data, AbstractNode *parent = nullptr)
        : AbstractNode(parent), m_data(data)
    {
        for (size_t i = 0; i < m_data->size(); i++)
        {
            detail::addNodeOfTo((*m_data)[i], static_cast<AbstractNode&>(*this));
        }
    }

    virtual QString fieldName(size_t index) const override
    {
        return QString::number(index);
    }

    virtual QVariant data(int column) const override
    {
        if (column == 0)
        {
            return parentItem() ? parentItem()->fieldName(row()) : "ROOT";
        }
        if (m_data)
            return qt_utils::toQVariant(std::format("{} elements.", m_data->size()));
        return {};
    }

private:
    std::array<ElementType, size> *m_data;
};

// Specialization for vectors of any type
template <class ElementType>
class Node <std::vector<ElementType>> : public AbstractNode
{
public:
    Node(std::vector<ElementType> *data, AbstractNode *parent = nullptr)
        : AbstractNode(parent), m_data(data)
    {
        // Only add children if the vector has less than 256 items
        // (then it probably shouldn't be edited by hand)
        if (m_data->size() < 256)
        {
            for (size_t i = 0; i < m_data->size(); i++)
            {
                detail::addNodeOfTo((*m_data)[i], static_cast<AbstractNode&>(*this));
            }
        }
    }

    virtual QString fieldName(size_t index) const override
    {
        return QString::number(index);
    }

    virtual QVariant data(int column) const override
    {
        if (column == 0)
        {
            return parentItem() ? parentItem()->fieldName(row()) : "ROOT";
        }
        if (m_data)
            return qt_utils::toQVariant(std::format("{} elements.", m_data->size()));
        return {};
    }

    virtual bool isInsertable() const override { return true; }

    virtual const std::vector<std::string_view> &getInsertOptions() const override
    {
        if constexpr (detail::is_variant_v<ElementType>)
        {
            static const std::vector<std::string_view> names = detail::getVariantTypesNames<ElementType>();
            return names;
        }
        else if constexpr (requires { { ElementType::getClassName() } -> std::same_as<std::string_view>; })
        {
            static const std::vector<std::string_view> names = { ElementType::getClassName() };
            return names;
        }
        else
        {
            static const std::vector<std::string_view> names = { typeid(ElementType).name() };
            return names;
        }
    }

    virtual bool insertChildren(int position, int count, size_t insertOptionIndex) override
    {
        if (position < 0 || position > qsizetype(m_children.size()))
            return false;

        for (int row = 0; row < count; ++row)
        {
            if constexpr (detail::is_variant_v<ElementType>)
            {
                ElementType variant;
                detail::setTypeOnVariant<0>(variant, insertOptionIndex);
                m_data->insert(m_data->cbegin() + position, variant);
            }
            else
            {
                m_data->insert(m_data->cbegin() + position, ElementType{});
            }
            // m_children.insert(m_children.cbegin() + position,
            //   std::make_unique<Node<std::remove_reference_t<decltype((*m_data)[row])>>>(
            //       &(*m_data)[position],
            //       this
            //   )
            // );
        }

        m_children.clear();
        for (size_t i = 0; i < m_data->size(); i++)
        {
            detail::addNodeOfTo((*m_data)[i], static_cast<AbstractNode&>(*this));
        }

        return true;
    }

    virtual bool removeChildren(int position, int count) override
    {
        if (position < 0 || position + count > qsizetype(m_children.size()))
            return false;

        for (int row = 0; row < count; ++row)
        {
            m_data->erase(m_data->cbegin() + position);
        }

        m_children.clear();
        for (size_t i = 0; i < m_data->size(); i++)
        {
            detail::addNodeOfTo((*m_data)[i], static_cast<AbstractNode&>(*this));
        }

        return true;
    }

private:
    std::vector<ElementType> *m_data;
};

/* StructModel {{{ */

template <typename T>
class StructModel : public QAbstractItemModel
{
public:
    explicit StructModel(T &refToStruct)
        : m_refToStruct(refToStruct)
    {
        m_rootItem = std::make_unique<Node<T>>(&refToStruct);
    }

    ~StructModel()
    {}

    void rebuild()
    {
        beginResetModel();
        m_rootItem = std::make_unique<Node<T>>(&m_refToStruct);
        endResetModel();
    }

    QVariant data(const QModelIndex &index, int role) const override
    {
        if (!index.isValid() || role != Qt::DisplayRole)
            return {};

        const AbstractNode *item = static_cast<const AbstractNode*>(index.internalPointer());
        return item->data(index.column());
    }

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override
    {
        if (!hasIndex(row, column, parent))
            return {};

        AbstractNode *parentItem = parent.isValid()
            ? static_cast<AbstractNode*>(parent.internalPointer())
            : m_rootItem.get();

        if (AbstractNode *childItem = parentItem->child(row))
            return createIndex(row, column, childItem);
        return {};
    }

    QModelIndex parent(const QModelIndex &index) const override
    {
        if (!index.isValid())
            return {};

        AbstractNode *childItem = static_cast<AbstractNode*>(index.internalPointer());
        AbstractNode *parentItem = childItem ? childItem->parentItem() : nullptr;

        return (parentItem != m_rootItem.get() && parentItem != nullptr)
            ? createIndex(parentItem->row(), 0, parentItem) : QModelIndex{};
    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const override
    {
        if (parent.column() > 0)
            return 0;

        const AbstractNode *parentItem = parent.isValid()
            ? static_cast<const AbstractNode*>(parent.internalPointer())
            : m_rootItem.get();

        return parentItem->childCount();
    }

    int columnCount(const QModelIndex &parent = QModelIndex()) const override
    {
        if (parent.isValid())
            return static_cast<AbstractNode*>(parent.internalPointer())->columnCount();
        return m_rootItem->columnCount();
    }

    Qt::ItemFlags flags(const QModelIndex &index) const override
    {
        if (!index.isValid())
            return Qt::NoItemFlags;

        AbstractNode *childItem = static_cast<AbstractNode*>(index.internalPointer());

        if (childItem != nullptr && childItem->isEditable())
            return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
        else
            return QAbstractItemModel::flags(index);
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override
    {
        return orientation == Qt::Horizontal && role == Qt::DisplayRole
            ? (section == 0 ? "Name" : "Value") : QVariant{};
    }

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override
    {
        INFO("{}, {}: {}, {}", index.row(), index.column(), value.typeName(), value.toString().toStdString());
        AbstractNode *childItem = static_cast<AbstractNode*>(index.internalPointer());
        return childItem->setData(index.column(), value);
    }

    bool insertRows(int position, int rows, const QModelIndex &parent = {}) override
    {
        return insertRows(position, rows, parent, 0);
    }

    bool insertRows(int position, int rows, const QModelIndex &parent, size_t insertOption)
    {
        AbstractNode *parentItem = static_cast<AbstractNode*>(parent.internalPointer());
        beginRemoveRows(parent, 0, parentItem->childCount());
        endRemoveRows();
        beginInsertRows(parent, 0, parentItem->childCount() + rows);
        bool result = parentItem->insertChildren(position, rows, insertOption);
        endInsertRows();
        
        return result;
    }

    bool removeRows(int position, int rows, const QModelIndex &parent = {}) override
    {
        AbstractNode *parentItem = static_cast<AbstractNode*>(parent.internalPointer());
        beginRemoveRows(parent, 0, parentItem->childCount());
        endRemoveRows();
        beginInsertRows(parent, 0, parentItem->childCount() + rows);
        return parentItem->removeChildren(position, rows);
        endInsertRows();
    }

private:
    std::unique_ptr<AbstractNode> m_rootItem;
    T &m_refToStruct;
};

/* StructModel }}} */

/* StructEditorItemDelegate {{{ */

class StructEditorItemDelegate : public QStyledItemDelegate {
public:
    explicit StructEditorItemDelegate(QObject *parent = nullptr)
        : QStyledItemDelegate(parent) {}

    void setEditorData(QWidget *editor, const QModelIndex &index) const override
    {
        AbstractNode *childItem = static_cast<AbstractNode*>(index.internalPointer());
        if (QLineEdit *lineEdit = qobject_cast<QLineEdit*>(editor))
        {
            ASSERT(childItem->data(index.column()).userType() == QMetaType::QString);
            lineEdit->setText(childItem->data(index.column()).toString());
        }
        else if (QSpinBox *spinBox = qobject_cast<QSpinBox*>(editor))
        {
            ASSERT(childItem->data(index.column()).canConvert<int>());
            spinBox->setValue(childItem->data(index.column()).toInt());
        }
        else if (QDoubleSpinBox *spinBox = qobject_cast<QDoubleSpinBox*>(editor))
        {
            ASSERT(childItem->data(index.column()).canConvert<float>());
            spinBox->setValue(childItem->data(index.column()).toDouble());
        }
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override
    {
        AbstractNode *childItem = static_cast<AbstractNode*>(index.internalPointer());
        if (QLineEdit *lineEdit = qobject_cast<QLineEdit*>(editor))
        {
            ASSERT(strcmp(childItem->data(index.column()).typeName(), "QString") == 0);
            if (!model->setData(index, lineEdit->text()))
                WARN("error setting string");
        }
        else if (QSpinBox *spinBox = qobject_cast<QSpinBox*>(editor))
        {
            ASSERT(childItem->data(index.column()).canConvert<int>());
            if (!model->setData(index, spinBox->value()))
                WARN("error setting int");
        }
        else if (QDoubleSpinBox *spinBox = qobject_cast<QDoubleSpinBox*>(editor))
        {
            ASSERT(childItem->data(index.column()).canConvert<float>());
            if (!model->setData(index, spinBox->value()))
                WARN("error setting float");
        }
    }

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        AbstractNode *childItem = static_cast<AbstractNode*>(index.internalPointer());
        AbstractNode::Type type = childItem->type();

        switch (type) {
            case AbstractNode::Type::None:
            {
                return nullptr;
            }
            case AbstractNode::Type::String:
            {
                QLineEdit *editor = new QLineEdit(parent);
                return editor;
            }
            case AbstractNode::Type::Int:
            {
                QSpinBox *editor = new QSpinBox(parent);
                editor->setRange(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
                return editor;
            }
            case AbstractNode::Type::Float:
            {
                QDoubleSpinBox *editor = new QDoubleSpinBox(parent);
                editor->setRange(-std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
                return editor;
            }
            default:
                ASSERT(false);
                break;
        }

        return nullptr;
    }

};

/* StructEditorItemDelegate }}} */
