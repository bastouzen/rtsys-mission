/*
 * This defines the mission model item. It represents one item of the model tree.
 * Each one holds a reference to their parent and to their children. The parent
 * reference is weak so that it isn't responsible for deleting its parent. The
 * children reference is strong so that it is responsible for deleting them. It
 * holds the data that are displayed in the model tree view.
 */

#ifndef RTSYS_MISSION_ITEM_H
#define RTSYS_MISSION_ITEM_H

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

namespace Qt {
enum MyRoles {
    UserRoleFeature = UserRole + 100,
    UserRolePack,
    UserRoleWrapper,
};
} // namespace Qt

// ===
// === Class
// ============================================================================ //

class MissionItem
{
    Q_GADGET; // needed for Q_ENUM

    static const int COLUMNS = 2;

  public:
    typedef google::protobuf::Message Protobuf;

    struct Wrapper {
        Protobuf *pointer;
    };

    enum FeatureFlag {

        // Feature Base Type
        kUndefined = 0,
        kMission = 1 << 0,
        kDevice = 1 << 1,
        kCollection = 1 << 2,
        kLine = 1 << 3,
        kPoint = 1 << 4,

        // Feature Type Interpreted
        kRail = 1 << 5,
        kSegment = 1 << 6,
        kScenario = 1 << 7,
        kRoute = 1 << 8,
        kFamily = 1 << 9,

        // Feature Type Action
        kDelete = 1 << 10,
        kEdit = 1 << 11,
        kSwap = 1 << 12,
    };
    Q_ENUM(FeatureFlag)                    // needed for QMetaEnum::fromType
    Q_DECLARE_FLAGS(Features, FeatureFlag) // needed for QFlags

    static Wrapper wrap(Protobuf *protobuf)
    {
        MissionItem::Wrapper wrapper;
        wrapper.pointer = protobuf;
        return wrapper;
    }
    static QByteArray pack(const Protobuf &protobuf);

    explicit MissionItem(MissionItem *parent = nullptr);
    ~MissionItem();

    void removeChild(const int row);
    void insertChild(const int row = -1);
    QVariant data(int role, const int column = 0) const;
    bool setData(const QVariant &value, int role);
    Features supportedFeatures() const;
    Qt::ItemFlags supportedItemFlags() const;

    // Getters and Setters
    MissionItem *parent() { return _parent; }
    MissionItem *child(int row) { return _childs.at(row); }
    int countChild() const { return _childs.count(); }
    int column() const { return COLUMNS; }
    int row() const { return _parent ? _parent->_childs.indexOf(const_cast<MissionItem *>(this)) : 0; }

  private:
    QVariant icon() const;
    Features feature() const;
    Features collection() const;
    void setDataFromFeature(const Features feature);
    bool setDataFromProtobuf(const QByteArray &packed);
    MissionItem *_parent;
    Protobuf *_protobuf;
    QVector<MissionItem *> _childs;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(MissionItem::Features) // needed for QFlags misc function
Q_DECLARE_METATYPE(MissionItem::Wrapper);            // needed for QVariant<MissionItem::Wrapper>

#endif // RTSYS_MISSION_ITEM_H
