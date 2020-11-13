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

// ===
// === Class
// ============================================================================ //

class ModelBacken
{
  public:
    enum Component { kMission, kCollection, kElement, kPoint, kRail, kSegment, kNoComponent };
    enum Collection { kScenario, kRoute, kFamily };
    enum Action { kDelete, kAddCollection, kAddPoint, kAddRail, kAddSegment };

    static Component component(const google::protobuf::Message *protobuf);
    static QVector<QVariant> data(const google::protobuf::Message *protobuf);

    explicit ModelBacken(google::protobuf::Message *protobuf = nullptr, ModelItem *item = nullptr);

    QVariant icon() const;
    unsigned int authorizedAction() const;
    bool isActionAuthorized(const Action action) const { return isActionAuthorized(action, authorizedAction()); }
    bool isActionAuthorized(const Action action, const unsigned int mask) const { return (mask >> action) & 1; }
    void removeRow(const int row);
    void clear();
    google::protobuf::Message *appendRow(const Action action);

  private:
    Component parentComponent() const;
    Component component() const;
    Collection collection() const;
    google::protobuf::Message *_protobuf;
    ModelItem *_item;
};

#endif // RTSYS_MISSION_MODEL_BACKEND_H
