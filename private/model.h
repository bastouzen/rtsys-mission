#ifndef RTSYS_MISSION_MODEL_H
#define RTSYS_MISSION_MODEL_H

// ===
// === Include
// ============================================================================ //

#include "private/backend.h"

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

// ===
// === Class
// ============================================================================ //

// This defines the mission item. It represents one item of the mission tree.
// Each one holds a reference to their parent and to their children. The parent
// reference is weak so that it isn't responsible for deleting it. The children
// reference is strong so that it is responsible for deleting them. It holds
// data '_data', these data are displayed in the tree view.
// The backend is responsible of managing the protobuf underlying data.
class MissionItem
{
  public:
    explicit MissionItem(const QVector<QVariant> &data, google::protobuf::Message *protobuf = nullptr,
                         MissionItem *parent = nullptr);
    ~MissionItem();

    void appendChild(MissionItem *child) { _childs.append(child); }
    void insertChild(int row, MissionItem *child) { _childs.insert(row, child); }
    void removeChild(int row);
    MissionItem *child(int row);
    const QVector<MissionItem *> &childs() const { return _childs; }
    int childCount() const { return _childs.count(); }
    int columnCount() const { return _data.count(); }
    QVariant data(int column) const;
    int row() const;

    MissionItem *parent() { return _parent; }
    MissionBackend &backend() { return _backend; }

  private:
    QVector<QVariant> _data;
    MissionItem *_parent;
    MissionBackend _backend;
    QVector<MissionItem *> _childs;
};


// This defines the mission model. The mission model is represented as a tree, each
// element of the tree (item) is linked to the root item in either parent or child
// relationship. The items of the tree are instance of 'MissionItem'. In other words
// the mission model holds all its data through the root item.
class MissionModel : public QAbstractItemModel
{
    Q_OBJECT

  public:
    explicit MissionModel(QObject *parent = nullptr);
    ~MissionModel();

    MissionItem *root() { return _root; }

    // These methods override the abstraction item model.
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;

    //setData()

    template <class T>
    void insertRow(int row, const QModelIndex &parent, T *protobuf = nullptr);
    void removeRow(int row, const QModelIndex &parent);
    MissionItem *item(const QModelIndex &index) const;

  private:
    QModelIndex index(MissionItem *item, int column) const;
    MissionItem *_root;
};

// Create then inserts an item specified by the given row and parent index
// When the parent index isn't valid it means that we try inserting top-level
// item so we set the parent item to the root item.
template <class T>
void MissionModel::insertRow(int row, const QModelIndex &parent, T *protobuf)
{
    auto *parent_item = parent.isValid() ? static_cast<MissionItem *>(parent.internalPointer()) : _root;

    beginInsertRows(parent, row, row + 1);
    parent_item->insertChild(row, new MissionItem({QString::fromStdString(protobuf->GetDescriptor()->name()),
                                                   QString::fromStdString(protobuf->name())},
                                                  protobuf, parent_item));
    endInsertRows();
}

#endif // RTSYS_MISSION_MODEL_H
