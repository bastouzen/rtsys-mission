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

namespace Qt {
enum MyRoles {
    UserRoleItemComponent = UserRole + 1, // Define role for the item backend component
    UserRoleItemProtobuf                  // Define role for the item backend protobuf data
};
} // namespace Qt

class ModelItem
{
    static const int COLUMN_COUT = 2;

  public:
    explicit ModelItem(google::protobuf::Message *protobuf = nullptr, ModelItem *parent = nullptr);
    ~ModelItem();

    // void insertRow(const int row, google::protobuf::Message *protobuf = nullptr);
    void appendRow(google::protobuf::Message *protobuf);
    void appendRow(const ModelBacken::Component component);
    void removeRow(const int row);
    //    void insertRow(const int row);
    //    void insertRow(const int row, const google::protobuf::Message &protobuf);

    // Getters and Setters
    void appendChild(ModelItem *child) { _childs.append(child); }
    ModelItem *child(int row);
    int childCount() const { return _childs.count(); }
    int columnCount() const { return COLUMN_COUT; }
    // QVariant data(int column) const;
    int row() const;
    //    bool setData(int column, const QVariant &value);
    ModelItem *parent() { return _parent; }
    ModelBacken &backend() { return _backend; }

    friend class ModelBacken;

  private:
    ModelItem *_parent;
    ModelBacken _backend;
    QVector<ModelItem *> _childs;
};

#endif // RTSYS_MISSION_MODEL_ITEM_H
