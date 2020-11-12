// ===
// === Include
// ============================================================================ //

#include "mission/mission_model.h"
#include "mission/mission_model_item.h"
#include "protobuf/mission.pb.h"

#include <QDebug>
#include <QIcon>

#define CastToItem(index) static_cast<MissionItem *>(index.internalPointer())

// ===
// === Class
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

// Returns the item flags for the given index.
Qt::ItemFlags MissionModel::flags(const QModelIndex &index) const
{
    return index.isValid() ? QAbstractItemModel::flags(index) : Qt::NoItemFlags;
}

// Returns the data stored under the given role for the specified index.
QVariant MissionModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) return QVariant();

    auto *item = CastToItem(index);

    if (role == Qt::DisplayRole) {
        return item->data(index.column());
    }

    if (role == Qt::DecorationRole) {
        if (index.column() == 0) {
            return item->backend().icon();
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
    return parent.isValid() ? CastToItem(parent)->columnCount() : _root->columnCount();
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

// Creates then returns the index specified by the given item and column.
QModelIndex MissionModel::index(MissionItem *item, int column) const
{
    if (!item || (item == _root)) {
        return QModelIndex(); // the root has no valid index model.
    }
    return createIndex(item->row(), column, item);
}

// Returns the item specified by the given index.
MissionItem *MissionModel::item(const QModelIndex &index) const
{
    return index.isValid() ? CastToItem(index) : nullptr;
}

// Creates then inserts an item specified by the given row and parent index.
// When the parent index isn't valid it means that we try inserting top-level
// item so we set the parent item to the root item.
void MissionModel::appendRow(const QModelIndex &parent, google::protobuf::Message *protobuf)
{
    auto *parent_item = parent.isValid() ? static_cast<MissionItem *>(parent.internalPointer()) : _root;

    beginInsertRows(parent, parent.row(), parent.row() + 1);
    parent_item->appendRow(protobuf);
    endInsertRows();
}

//// Remove the index specified by the given row and parent index. When the
//// parent index isn't valid it means that we try removing top-level item so
//// we set the parent item to the root item.
// void MissionModel::removeRow(int row, const QModelIndex &parent)
//{
//    auto *parent_item = parent.isValid() ? CastToItem(parent) : _root;

//    if (rowCount(parent) && rowCount(parent) >= row) {
//        beginRemoveRows(parent, parent.row(), parent.row() + 1);
//        // parent_item->removeChild(row);
//        endRemoveRows();
//    }
//}
