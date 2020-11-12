#ifndef RTSYS_MISSION_ITEM_H
#define RTSYS_MISSION_ITEM_H

// ===
// === Include
// ============================================================================ //

#include "mission/backend.h"

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

    void appendRow(google::protobuf::Message *protobuf);
    void removeRow(int row);

    // Getters and Setters
    void appendChild(MissionItem *child) { _childs.append(child); }
    MissionItem *child(int row);
    int childCount() const { return _childs.count(); }
    int columnCount() const { return _data.count(); }
    QVariant data(int column) const;
    int row() const;
    MissionItem *parent() { return _parent; }
    MissionBackend &backend() { return _backend; }

    friend class MissionBackend;

  private:
    QVector<QVariant> _data;
    MissionItem *_parent;
    MissionBackend _backend;
    QVector<MissionItem *> _childs;
};

#endif // RTSYS_MISSION_ITEM_H
