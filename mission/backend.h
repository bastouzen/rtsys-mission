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
    enum Component { kMission, kCollection, kElement, kPoint, kRail, kSegment, kNoComponent, kDelete };
    enum Collection { kScenario, kRoute, kFamily };

    static Component component(const google::protobuf::Message *protobuf);

    explicit ModelBacken(google::protobuf::Message *protobuf = nullptr, ModelItem *item = nullptr);

    QVariant icon() const;
    unsigned int authorization() const;
    bool isAuthorized(const Component component) const { return isAuthorized(component, authorization()); }
    bool isAuthorized(const Component component, const unsigned int mask) const { return (mask >> component) & 1; }
    Component component() const;
    bool removeRow(const int row);
    google::protobuf::Message *appendRow(const Component new_component);
    void clear();
    QVariant data(const int column) const;
    bool setData(int column, const QVariant &value);
    google::protobuf::Message *protobuf() { return _protobuf; }
    bool moveLastAt(const int row);

  private:
    Component parentComponent() const;
    Collection collection() const;
    google::protobuf::Message *_protobuf;
    ModelItem *_item;
};

#endif // RTSYS_MISSION_MODEL_BACKEND_H
