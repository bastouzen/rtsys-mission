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

class MissionItem;

// ===
// === Class
// ============================================================================ //

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

    // setData()
    void appendRow(const QModelIndex &parent, google::protobuf::Message *protobuf = nullptr);
    // void removeRow(int row, const QModelIndex &parent);
    MissionItem *item(const QModelIndex &index) const;

  private:
    QModelIndex index(MissionItem *item, int column) const;
    MissionItem *_root;
};

#endif // RTSYS_MISSION_MODEL_H
