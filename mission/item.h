/*
 * This defines the model item object. It represents one item of the model tree.
 * Each one holds a reference to their parent and to their children. The parent
 * reference is weak so that it isn't responsible for deleting its parent. The
 * children reference is strong so that it is responsible for deleting them. It
 * holds the data that are displayed in the model tree view.
 *
 * The backend is responsible of managing the underlying protobuf message.
 */

#ifndef RTSYS_MISSION_MODEL_ITEM_H
#define RTSYS_MISSION_MODEL_ITEM_H

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

class ModelItem
{
  public:
    explicit ModelItem(const QVector<QVariant> &data, google::protobuf::Message *protobuf = nullptr,
                       ModelItem *parent = nullptr);
    ~ModelItem();

    void appendRow(google::protobuf::Message *protobuf);
    void appendRow(const ModelBacken::Action action);
    void removeRow(int row);

    // Getters and Setters
    void appendChild(ModelItem *child) { _childs.append(child); }
    ModelItem *child(int row);
    int childCount() const { return _childs.count(); }
    int columnCount() const { return _data.count(); }
    QVariant data(int column) const;
    int row() const;
    bool setData(int column, const QVariant &value);
    ModelItem *parent() { return _parent; }
    ModelBacken &backend() { return _backend; }

    friend class ModelBacken;

  private:
    QVector<QVariant> _data;
    ModelItem *_parent;
    ModelBacken _backend;
    QVector<ModelItem *> _childs;
};

#endif // RTSYS_MISSION_MODEL_ITEM_H
