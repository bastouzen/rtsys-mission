// ===
// === Include
// ============================================================================ //

#include "mission/model.h"
#include "mission/item.h"
#include "protobuf/misc/misc_cpp.h"
#include "protobuf/mission.pb.h"

#include <QDataStream>
#include <QLoggingCategory>
#include <QMimeData>

// ===
// === Define
// ============================================================================ //

Q_LOGGING_CATEGORY(LC_RMM, "rtsys.mission.model")

#define _Item(index) static_cast<MissionItem *>(index.internalPointer())
#define _ItemOrRoot(index) (index.isValid() ? _Item(index) : _root)

// ===
// === Class
// ============================================================================ //

MissionModel::MissionModel(QObject *parent)
    : QAbstractItemModel(parent)
    , _root(new MissionItem())
{
}

MissionModel::~MissionModel()
{
    delete _root;
}

// ====================================
// Read-only Model
// ====================================

// Returns the data stored under the given role for the specified index.
QVariant MissionModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) return QVariant();

    return _Item(index)->data(role, index.column());
}

// Returns the data for the given role and section in the header with the specified orientation.
QVariant MissionModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    const QVector<QVariant> HEADER_DATA = {tr("Feature"), tr("Name")};
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return HEADER_DATA[section];
    }
    return QVariant();
}

// Returns the number of rows under the given parent index. When the parent
// index is valid it means that rowCount is returning the number of children
// of parent index.
int MissionModel::rowCount(const QModelIndex &parent) const
{
    return _ItemOrRoot(parent)->countChild();
}

// Returns the number of columns for the children of the given parent index.
int MissionModel::columnCount(const QModelIndex &parent) const
{
    return _ItemOrRoot(parent)->column();
}

// Creates then returns the index specified by the given row, column and parent index.
QModelIndex MissionModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent)) return QModelIndex();

    auto *child = _ItemOrRoot(parent)->child(row);
    if (child) {
        return createIndex(row, column, child);
    }
    return QModelIndex();
}

// Creates then returns the parent index of the child index.
QModelIndex MissionModel::parent(const QModelIndex &child) const
{
    if (child.isValid()) {
        return index(_Item(child)->parent(), 0);
    }
    return QModelIndex();
}

// ====================================
// Editing and Resizing Model
// ====================================

// Returns the item flags for the given index.
Qt::ItemFlags MissionModel::flags(const QModelIndex &index) const
{
    // Qt::ItemIsSelectable|Qt::ItemIsEnabled;
    if (!index.isValid()) return Qt::NoItemFlags;

    if (index.column() > 0)
        return Qt::ItemIsEnabled | Qt::ItemIsEditable;
    else
        return Qt::ItemIsEnabled | _Item(index)->supportedItemFlags();
}

// Sets the data for the specified value and role. Returns true if successful
// otherwise returns false.
bool MissionModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid()) return false;

    if (_Item(index)->setData(value, role)) {
        emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
        return true;
    }
    return false;
}

// Removes the item specified by the given row and parent index.
bool MissionModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (rowCount(parent) && rowCount(parent) >= row) {
        beginRemoveRows(parent, row, row + count - 1);
        for (int i = 0; i < count; i++) {
            _ItemOrRoot(parent)->removeChild(row);
        }
        endRemoveRows();
        return true;
    }

    return QAbstractItemModel::removeRows(row, count, parent); // returns false
}

// Insert an item specified by the given row and parent index.
bool MissionModel::insertRows(int row, int count, const QModelIndex &parent)
{
    beginInsertRows(parent, row, row + count - 1);
    for (int i = 0; i < count; i++) {
        _ItemOrRoot(parent)->insertChild(row);
    }
    endInsertRows();

    return true;
}

// ====================================
// Drag & Drop Model
// ====================================

// Returns the drop actions supported by this view.
Qt::DropActions MissionModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

// Returns a map with values for all predefined roles in the model for the item
// at the given index.
QMap<int, QVariant> MissionModel::itemData(const QModelIndex &index) const
{
    // auto roles = QAbstractItemModel::itemData(index);
    QMap<int, QVariant> roles;
    for (auto i : {Qt::UserRoleFeature, Qt::UserRolePack}) {
        auto var = data(index, i);
        if (var.isValid()) roles.insert(i, var);
    }
    return roles;
}

// Returns true if the model can accept a drop of the data.
bool MissionModel::canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column,
                                   const QModelIndex &parent) const
{
    if (!parent.isValid()) return false;
    if (!QAbstractItemModel::canDropMimeData(data, action, row, column, parent)) return false;

    // Retrieve the flags identifiers of the drags indexes holds into the mimeData container.
    auto getDragMask = [&]() {
        QByteArray encoded = data->data(mimeTypes().first());
        QDataStream stream(&encoded, QIODevice::ReadOnly);
        MissionItem::Features mask;
        while (!stream.atEnd()) {
            int row, column;
            QMap<int, QVariant> roles;
            stream >> row >> column >> roles;
            mask |= MissionItem::Features(roles[Qt::UserRoleFeature].toInt());
        }
        return mask;
    };

    return getDragMask() & item(parent)->supportedFeatures();
}

// ============================================================================ //

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
    return index.isValid() ? _Item(index) : nullptr;
}

// Reverses the index's children order of the specifed index.
bool MissionModel::reverseIndexChildren(const QModelIndex &index)
{
    if (!index.isValid()) return false;

    // Backup children indexes
    auto *item = _Item(index);
    QModelIndexList children;
    auto count = item->countChild();
    for (int i = 0; i < count; i++) {
        children << this->index(item->child(i));
    }
    auto data = mimeData(children);

    // Remove all index's children then insert 'count' into index's child
    removeRows(0, count, index);
    insertRows(0, count, index);

    // Restore children indexes (reverse order)
    QByteArray encoded = data->data(mimeTypes().first());
    QDataStream stream(&encoded, QIODevice::ReadOnly);
    QVector<int> rows;
    QVector<QMap<int, QVariant>> roles;
    while (!stream.atEnd()) {
        int row, column;
        QMap<int, QVariant> map;
        stream >> row >> column >> map;
        rows << row;
        roles << map;
    }
    for (int i = 0; i < count; i++) {
        setItemData(this->index(i, 0, index), roles.at(count - 1 - i));
    }

    return true;
}
