// ===
// === Include
// ============================================================================ //

#include "mission/model.h"
#include "mission/item.h"
#include "protobuf/mission.pb.h"

#include <QDebug>

// ===
// === Define
// ============================================================================ //

#define CastToItem(index) static_cast<ModelItem *>(index.internalPointer())

// ===
// === Class
// ============================================================================ //

MissionModel::MissionModel(QObject *parent)
    : QAbstractItemModel(parent)
    , _root(new ModelItem({tr("Component"), tr("Name")}))
{
}

MissionModel::~MissionModel()
{
    delete _root;
}

// =============================
// Read-only Model
// =============================

// Returns the data stored under the given role for the specified index.
QVariant MissionModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) return QVariant();

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        return CastToItem(index)->data(index.column());
    }

    if (role == Qt::DecorationRole) {
        if (index.column() == 0) {
            return CastToItem(index)->backend().icon();
        };
    }

    return QVariant();
}

// Returns the data for the given role and section in the header with the specified orientation.
QVariant MissionModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return _root->data(section);
    }
    return QVariant();
}

// Returns the number of rows under the given parent index. When the parent
// index is valid it means that rowCount is returning the number of children
// of parent index.
int MissionModel::rowCount(const QModelIndex &parent) const
{
    return (parent.isValid() ? CastToItem(parent) : _root)->childCount();
}

// Returns the number of columns for the children of the given parent index.
int MissionModel::columnCount(const QModelIndex &parent) const
{
    return (parent.isValid() ? CastToItem(parent) : _root)->columnCount();
}

// Creates then returns the index specified by the given row, column and parent index.
QModelIndex MissionModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent)) return QModelIndex();

    auto *parent_item = parent.isValid() ? CastToItem(parent) : _root;
    auto *child_item = parent_item->child(row);
    if (child_item) {
        return createIndex(row, column, child_item);
    }
    return QModelIndex();
}

// Creates then returns the parent index of the child index.
QModelIndex MissionModel::parent(const QModelIndex &child) const
{
    if (child.isValid()) {
        return index(CastToItem(child)->parent(), 0);
    }
    return QModelIndex();
}

// =============================
// Editing and Resizing Row-only
// =============================

// Returns the item flags for the given index.
Qt::ItemFlags MissionModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) return Qt::NoItemFlags;

    if (index.column() > 0)
        return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
    else
        return QAbstractItemModel::flags(index);
}

bool MissionModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole) return false;

    bool result = CastToItem(index)->setData(index.column(), value);
    if (result) {
        emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
    }
    return result;
}

// ============================================================================ //

// Creates then returns the index specified by the given item and column.
QModelIndex MissionModel::index(ModelItem *item, int column) const
{
    if (!item || (item == _root)) {
        return QModelIndex(); // the root has no valid index model.
    }
    return createIndex(item->row(), column, item);
}

// Returns the item specified by the given index.
ModelItem *MissionModel::item(const QModelIndex &index) const
{
    return index.isValid() ? CastToItem(index) : nullptr;
}

// Appends an item to the specified parent children for the specified underlying
// protobug message.
void MissionModel::appendRow(const QModelIndex &parent, google::protobuf::Message *protobuf)
{
    beginInsertRows(parent, rowCount(parent), rowCount(parent) + 1);
    (parent.isValid() ? CastToItem(parent) : _root)->appendRow(protobuf);
    endInsertRows();
}

// Appends an item to the specified parent children for the specified action.
void MissionModel::appendRow(const QModelIndex &parent, const int action)
{
    beginInsertRows(parent, rowCount(parent), rowCount(parent) + 1);
    (parent.isValid() ? CastToItem(parent) : _root)->appendRow(static_cast<ModelBacken::Action>(action));
    endInsertRows();
}

// Removes the item specified by the given row and parent index.
void MissionModel::removeRow(int row, const QModelIndex &parent)
{
    if (rowCount(parent) && rowCount(parent) >= row) {
        beginRemoveRows(parent, row, row + 1);
        (parent.isValid() ? CastToItem(parent) : _root)->removeRow(row);
        endRemoveRows();
    }
}
