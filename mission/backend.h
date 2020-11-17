/*
 * This defines the model backend object. The backend is responsible of managing
 * the underlying protobuf message. It provides some intefaces to the model for
 * getting and setting part of the underlying protobuf message (Add, Remove, ...)
 */

#ifndef RTSYS_MISSION_MODEL_BACKEND_H
#define RTSYS_MISSION_MODEL_BACKEND_H

// ===
// === Include
// ============================================================================ //

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

//namespace Qt {
//enum MyRoles {
//    UserRoleFlag = UserRole + 1,
//    UserRoleFlagDescription,
//    UserRoleFlagName,
//    UserRoleProtobufStream,
//};
//} // namespace Qt

// ===
// === Class
// ============================================================================ //

class ModelBacken
{
  public:
    typedef google::protobuf::Message Protobuf;

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

    static Flag flag(const Protobuf *protobuf);
    explicit ModelBacken(ModelItem *item = nullptr);

    bool removeRow(const int row);
    Protobuf *insertRow(const int row, const Flag new_flag);

    QVariant icon() const;
    unsigned int supportedFlags() const;
    bool canSupportFlag(const Flag flag, const unsigned int mask) const { return (mask >> flag) & 1; }
    bool canSupportFlag(const Flag flag) const { return canSupportFlag(flag, supportedFlags()); }
    bool canDropFlag(const Flag flag) const { return canSupportFlag(flag); }
    Flag flag() const;

    QVariant data(const int role) const;
    bool setData(const QVariant &value, const int role);
    void setDataProtobufPointer(Protobuf *protobuf) { _protobuf = protobuf; }

    Protobuf *protobuf() { return _protobuf; }

  private:
    Protobuf *appendRow(const Flag new_flag);
    Flag parentFlag() const;
    Flag collectionFlag() const;
    Protobuf *_protobuf;
    ModelItem *_item;
};

#endif // RTSYS_MISSION_MODEL_BACKEND_H
