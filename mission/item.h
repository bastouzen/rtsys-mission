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
    UserRoleFlag = UserRole + 1,
    UserRoleName,
    UserRolePack,
};
} // namespace Qt

// ===
// === Class
// ============================================================================ //

class MissionItem
{
    Q_GADGET

    static const int COLUMNS = 2;

  public:
    typedef google::protobuf::Message Protobuf;

    struct Wrapper {
        Protobuf *pointer;
    };

    enum Flag {
        kUndefined,
        kMission,
        kComponent,
        kCollection,
        kElement,
        kPoint,
        kRail,
        kSegment,
        kDelete,
        kScenario,
        kRoute,
        kFamily
    };
    Q_ENUM(Flag);

    static Flag flag(const Protobuf *protobuf);
    static QByteArray pack(const Protobuf &protobuf);
    static QSharedPointer<Protobuf> unpack(const QByteArray &array);

    explicit MissionItem(MissionItem *parent = nullptr);
    ~MissionItem();

    void removeChild(const int row);
    void insertChild(const int row = -1);

    unsigned int supportedFlags() const;
    bool isFlagSupported(const Flag flag, const unsigned int mask) const { return (mask >> flag) & 1; }
    bool isFlagSupported(const Flag flag) const { return isFlagSupported(flag, supportedFlags()); }

    // Getters and Setters
    MissionItem *parent() { return _parent; }
    Protobuf *protobuf() { return _protobuf; }
    MissionItem *child(int row) { return _childs.at(row); }
    int countChild() const { return _childs.count(); }
    int column() const { return COLUMNS; }
    int row() const { return _parent ? _parent->_childs.indexOf(const_cast<MissionItem *>(this)) : 0; }

    QVariant data(int role) const;
    QVariant icon() const;
    bool setData(const QVariant &value, int role);
    void setProtobuf(Protobuf *protobuf) { _protobuf = protobuf; }

  private:
    Flag parentFlag() const;
    Flag collectionFlag() const;
    void setDataFromFlag(const Flag new_flag);
    bool setDataFromProtobuf(Protobuf *protobuf);
    bool setDataFromProtobuf(const QByteArray stream);
    MissionItem *_parent;
    Protobuf *_protobuf;
    QVector<MissionItem *> _childs;
};

#endif // RTSYS_MISSION_MODEL_ITEM_H
