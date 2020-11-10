// ===
// === Include
// ============================================================================ //

#include "private/model.h"
#include "protobuf/mission.pb.h"

#include <QDebug>
#include <QIcon>

#define CastToItem(index) static_cast<MissionItem *>(index.internalPointer())

// ===
// === Class
// ============================================================================ //

MissionItem::MissionItem(const QVector<QVariant> &data, google::protobuf::Message *protobuf, MissionItem *parent)
    : _data(data)
    , _parent(parent)
    , _backend(protobuf, this)
{
}

MissionItem::~MissionItem()
{
    qDeleteAll(_childs);
}

void MissionItem::removeChild(int row)
{
    _childs.remove(row);
    _backend.remove(row);
}

MissionItem *MissionItem::child(int row)
{
    if (row < 0 || row >= _childs.size()) {
        return nullptr;
    }
    return _childs.at(row);
}

QVariant MissionItem::data(int column) const
{
    if (column < 0 || column >= _data.size()) {
        return QVariant();
    }
    return _data.at(column);
}

int MissionItem::row() const
{
    if (_parent) {
        return _parent->_childs.indexOf(const_cast<MissionItem *>(this));
    }
    return 0;
}

// ============================================================================ //

MissionModel::MissionModel(QObject *parent)
    : QAbstractItemModel(parent)
    , _root(new MissionItem({tr("Component"), tr("Name")}))
{
}

MissionModel::~MissionModel()
{
    delete _root;
}

// Returns the number of columns for the children of the given parent.
int MissionModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? CastToItem(parent)->columnCount() : _root->columnCount();
}

// Returns the data stored under the given role for the item referred to by the index.
QVariant MissionModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) return QVariant();

    auto *item = CastToItem(index);

    if (role == Qt::DisplayRole) {
        return item->data(index.column());
    }

    if (role == Qt::DecorationRole) {
        if (index.column() == 0) {
            return item->backend()->icon();
        };
    }

    return QVariant();
}

// Returns the item flags for the given index.
Qt::ItemFlags MissionModel::flags(const QModelIndex &index) const
{
    return index.isValid() ? QAbstractItemModel::flags(index) : Qt::NoItemFlags;
}

// Returns the data for the given role and section in the header with the specified orientation.
QVariant MissionModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return _root->data(section);
    }
    return QVariant();
}

// Returns the index of the item in the model specified by the given row, column and parent index.
QModelIndex MissionModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent)) return QModelIndex();

    auto *parent_ = parent.isValid() ? CastToItem(parent) : _root;
    auto *child = parent_->child(row);
    if (child) {
        return createIndex(row, column, child);
    }
    return QModelIndex();
}

// Returns the parent of the child model item.
QModelIndex MissionModel::parent(const QModelIndex &child) const
{
    if (child.isValid()) {
        return index(CastToItem(child)->parent(), 0);
    }
    return QModelIndex();
}

// Returns the item in the model specified by the given index of the item in the model.
MissionItem *MissionModel::item(const QModelIndex &index) const
{
    return index.isValid() ? CastToItem(index) : nullptr;
}

// Returns the index model specified by the given item modem and column.
QModelIndex MissionModel::index(MissionItem *item, int column) const
{
    if (!item || (item == _root)) {
        return QModelIndex(); // the root has no valid model index.
    }
    return createIndex(item->row(), column, item);
}

// Returns the number of rows under the given parent. When the parent is valid
// it means that rowCount is returning the number of children of parent.
int MissionModel::rowCount(const QModelIndex &parent) const
{
    return (parent.isValid() ? CastToItem(parent) : _root)->childCount();
}

// This removes an item from the model specified by the row and the parent.
void MissionModel::removeRow(int row, const QModelIndex &parent)
{
    auto *parent_ = parent.isValid() ? CastToItem(parent) : _root;

    if (rowCount(parent) && rowCount(parent) >= row) {
        beginRemoveRows(parent, parent.row(), parent.row() + 1);
        parent_->removeChild(row);
        parent_ = nullptr;
        endRemoveRows();
    }
}
