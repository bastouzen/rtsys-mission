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
        return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | QAbstractItemModel::flags(index);
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

bool MissionModel::insertRows(int row, int count, const QModelIndex &parent)
{
    if (count != 1) {
        qDebug() << "Ein Bug Problem";
        return false;
    }

    beginInsertRows(parent, rowCount(parent), rowCount(parent) + 1);
    //(parent.isValid() ? CastToItem(parent) : _root)->insertRow(row);
    endInsertRows();
    return true;
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
    QMimeData *mime_data = new QMimeData;
    QByteArray encoded_data;
    QDataStream stream(&encoded_data, QIODevice::WriteOnly);

    for (const QModelIndex &index : indexes) {
        if (index.isValid()) {
            stream << QString::fromStdString(
                rtsys::protobuf::misc::serializeDelimitedToString(*CastToItem(index)->backend().protobuf()));
        }
    }

    qDebug() << "::mimeData" << mime_data;
    mime_data->setData(mimeTypes().first(), encoded_data);
    return mime_data;
}

bool MissionModel::canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column,
                                   const QModelIndex &parent) const
{
    auto result = QAbstractItemModel::canDropMimeData(data, action, row, column, parent);

    // Need to check if the target will be accepted
    qDebug() << "::canDropMimeData" << result << row << column << parent;
    return result;
}

bool MissionModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column,
                                const QModelIndex &parent)
{
    // auto result = QAbstractItemModel::dropMimeData(data, action, row, column, parent);
    // qDebug() << "::dropMimeData" << result;
    // return result;

    // check if the action is supported
    if (!data || !(action == Qt::CopyAction || action == Qt::MoveAction)) return false;
    // check if the format is supported
    QStringList types = mimeTypes();
    if (types.isEmpty()) return false;
    QString format = types.at(0);
    if (!data->hasFormat(format)) return false;
    if (row > rowCount(parent)) row = rowCount(parent);
    if (row == -1) row = rowCount(parent);
    if (column == -1) column = 0;
    // decode and insert
    QByteArray encoded = data->data(format);
    QDataStream stream(&encoded, QIODevice::ReadOnly);
    return decodeData(row, column, parent, stream);
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
    QModelIndex parent;
    beginInsertRows(parent, rowCount(parent), rowCount(parent) + 1);
    _root->appendRow(protobuf);
    endInsertRows();
}

// Appends an item to the specified parent children for the specified component.
void MissionModel::appendRow(const QModelIndex &parent, const int component)
{
    if (!parent.isValid()) return;

    beginInsertRows(parent, rowCount(parent), rowCount(parent) + 1);
    CastToItem(parent)->appendRow(static_cast<ModelBacken::Component>(component));
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
