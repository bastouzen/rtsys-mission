// ===
// === Include
// ============================================================================ //

#include "mission/model.h"
#include "mission/item.h"
#include "protobuf/misc/misc_cpp.h"
#include "protobuf/mission.pb.h"

#include <QDataStream>
#include <QDebug>
#include <QMimeData>

// ===
// === Define
// ============================================================================ //

#define CLASSNAME "MissionModel ::"

#define CastToItem(index) static_cast<ModelItem *>(index.internalPointer())

// ===
// === Class
// ============================================================================ //

MissionModel::MissionModel(QObject *parent)
    : QAbstractItemModel(parent)
    , _root(new ModelItem())
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
        if (index.column() == 0) {
            return CastToItem(index)->data(Qt::UserRoleFlagStr);
        }
        if (index.column() == 1) {
            return CastToItem(index)->data(Qt::UserRoleName);
        }
    }

    if (role == Qt::DecorationRole) {
        if (index.column() == 0) {
            return CastToItem(index)->data(Qt::DecorationRole);
        }
    }

    if (role == Qt::UserRoleFlagId) {
        return CastToItem(index)->data(Qt::UserRoleFlagId);
    }

    if (role == Qt::UserRoleWrapper) {
        return CastToItem(index)->data(Qt::UserRoleWrapper);
    }

    return QVariant();
}

// Returns the data for the given role and section in the header with the specified orientation.
QVariant MissionModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    const QVector<QVariant> HEADER_DATA = {tr("Component"), tr("Name")};
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
    return (parent.isValid() ? CastToItem(parent) : _root)->countChild();
}

// Returns the number of columns for the children of the given parent index.
int MissionModel::columnCount(const QModelIndex &parent) const
{
    return (parent.isValid() ? CastToItem(parent) : _root)->column();
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
        return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | QAbstractItemModel::flags(index);
}

// Sets the data for the specified value and role. Returns true if successful
// otherwise returns false.
bool MissionModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole) {
        if (CastToItem(index)->setData(value, Qt::UserRoleName)) {
            emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
            return true;
        }
    }

    return CastToItem(index)->setData(value, role);
}

// Removes the item specified by the given row and parent index.
bool MissionModel::removeRows(int row, int count, const QModelIndex &parent)
{
    Q_ASSERT_X(count == 1, CLASSNAME, "fail removing row");

    if (rowCount(parent) && rowCount(parent) >= row) {
        beginRemoveRows(parent, row, row + 1);
        (parent.isValid() ? CastToItem(parent) : _root)->removeChild(row);
        endRemoveRows();
        return true;
    }

    return QAbstractItemModel::removeRows(row, count, parent); // returns false
}

// Insert an item specified by the given row and parent index.
bool MissionModel::insertRows(int row, int count, const QModelIndex &parent)
{
    Q_ASSERT_X(count == 1, CLASSNAME, "fail inserting row");

    beginInsertRows(parent, row, row + 1);
    (parent.isValid() ? CastToItem(parent) : _root)->insertChild(row);
    endInsertRows();
    return true;

    return QAbstractItemModel::insertRows(row, count, parent); // returns false
}

// =============================
// Drag & Drop
// =============================

// Returns the drop actions supported by this view.
Qt::DropActions MissionModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

QMimeData *MissionModel::mimeData(const QModelIndexList &indexes) const
{
    if (indexes.count() <= 0) return 0;
    QStringList types = mimeTypes();
    if (types.isEmpty()) return 0;
    QMimeData *data = new QMimeData();
    QString format = types.at(0);
    QByteArray encoded;
    QDataStream stream(&encoded, QIODevice::WriteOnly);
    encodeData(indexes, stream);
    data->setData(format, encoded);

    qDebug() << "==============> CCCCCC";
    qDebug() << data;

    return data;
}

// Returns a map with values for all predefined roles in the model for the item
// at the given index.
QMap<int, QVariant> MissionModel::itemData(const QModelIndex &index) const
{
    auto roles = QAbstractItemModel::itemData(index);
    roles.insert(Qt::UserRoleFlagId, data(index, Qt::UserRoleFlagId));

    qDebug() << "==============================A";
    qDebug() << roles;
    qDebug() << "==============================B";
    return roles;
}

// Returns true if the model can accept a drop of the data.
bool MissionModel::canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column,
                                   const QModelIndex &parent) const
{
    if (!QAbstractItemModel::canDropMimeData(data, action, row, column, parent)) return false;

    qDebug() << "::canDropMimeData" << data << action << row << column << parent;
    auto child_index = index(row, column, parent);
    if (child_index.isValid()) {
        qDebug() << "===>" << parent.data(Qt::UserRoleFlagId) << "|" << child_index.data(Qt::UserRoleFlagId);
        return CastToItem(parent)->isFlagSupported(
            static_cast<ModelItem::Flag>(child_index.data(Qt::UserRoleFlagId).toInt()));
    }
    return false;
}

bool MissionModel::setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles)
{
    qDebug() << "::setItemData" << index << roles;
    return QAbstractItemModel::setItemData(index, roles);
}

// TODO
bool MissionModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column,
                                const QModelIndex &parent)
{
    qDebug() << "::dropMimeData" << data << action << row << column << parent;
    return QAbstractItemModel::dropMimeData(data, action, row, column, parent);

    //    QByteArray input = data->data(mimeTypes().first());
    //    QDataStream stream(&input, QIODevice::ReadOnly);
    //    int component;
    //    QByteArray buffer;
    //    stream >> component;
    //    stream >> buffer;

    //    QScopedPointer<google::protobuf::Message> protobuf(
    //        ModelBacken::factory(static_cast<ModelBacken::Component>(component), buffer));
    //    qDebug() << protobuf->DebugString().data();

    //    auto result = QAbstractItemModel::dropMimeData(data, action, row, column, parent);
    //    qDebug() << "::dropMimeData" << row << column << parent << result << this->data(parent, Qt::DisplayRole);

    //    //insertRow(row, parent, *protobuf);
    //    // removeRow();

    //    return result;
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

// Appends an item to the root children for the specified underlying protobuf
// message.
void MissionModel::appendRow(google::protobuf::Message *protobuf)
{
    //    QModelIndex parent;
    //    beginInsertRows(parent, rowCount(parent), rowCount(parent) + 1);
    //    _root->appendRow(protobuf);
    //    endInsertRows();
}

// Appends an item to the specified parent children for the specified component.
void MissionModel::appendRow(const QModelIndex &parent, const int component)
{
    if (!parent.isValid()) return;

    //    beginInsertRows(parent, rowCount(parent), rowCount(parent) + 1);
    //    CastToItem(parent)->appendRow(static_cast<ModelBacken::Flag>(component));
    //    endInsertRows();
}
