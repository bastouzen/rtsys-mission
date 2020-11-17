/*
 * This defines the mission model. The mission model is represented as a tree, each
 * element of the tree (item) is linked to the root item in either parent or child
 * relationship. The items of the tree are instance of 'ModelItem'. In other words
 *the mission model holds all its data through the root item.
 */

#ifndef RTSYS_MISSION_MODEL_H
#define RTSYS_MISSION_MODEL_H

// ===
// === Include
// ============================================================================ //

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

// ===
// === Define
// ============================================================================ //

namespace google {
namespace protobuf {
class Message;
} // namespace protobuf
} // namespace google

class ModelItem;

// ===
// === Class
// ============================================================================ //

class MissionModel : public QAbstractItemModel
{
    Q_OBJECT

  public:
    explicit MissionModel(QObject *parent = nullptr);
    ~MissionModel();

    // These methods override the abstraction item model.
    // Reading Model
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;

    // Editing and Resizing Model
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    // Drag & Drop Model
    Qt::DropActions supportedDropActions() const override;
    // QMimeData *mimeData(const QModelIndexList &indexes) const override;
    bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column,
                         const QModelIndex &parent) const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column,
                      const QModelIndex &parent) override;

    QMap<int, QVariant> itemData(const QModelIndex &index) const override;
    bool setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles) override;

    ModelItem *root() { return _root; }
    ModelItem *item(const QModelIndex &index) const;
    QModelIndex index(ModelItem *item, int column = 0) const;
    void appendRow(google::protobuf::Message *protobuf);
    void appendRow(const QModelIndex &parent, const int component);
    // void removeRow(int row, const QModelIndex &parent);
    // void insertRow(int row, const QModelIndex &parent, const google::protobuf::Message &protobuf);

  private:
    ModelItem *_root;
};

#endif // RTSYS_MISSION_MODEL_H
