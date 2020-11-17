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
    typedef google::protobuf::Message Protobuf;
    enum Component { kMission, kCollection, kElement, kPoint, kRail, kSegment, kNoComponent, kDelete };
    enum Collection { kScenario, kRoute, kFamily };

    static Component component(const Protobuf *protobuf);
    static Protobuf *factory(const Component component, const QByteArray &stream);

    explicit ModelBacken(Protobuf *protobuf = nullptr, ModelItem *item = nullptr);

    bool removeRow(const int row);
    Protobuf *appendRow(const Component new_component);

    QVariant icon() const;
    unsigned int supportedComponent() const;
    bool canSupport(const Component component) const { return canSupport(component, supportedComponent()); }
    bool canSupport(const Component component, const unsigned int mask) const { return (mask >> component) & 1; }
    bool canDropComponent(const Component drop_component) const;
    Component component() const;


    QVariant data(const int column) const;
    bool setData(int column, const QVariant &value);
    Protobuf *protobuf() { return _protobuf; }

    bool moveUpLastRowAt(const int row);

  private:
    Component parentComponent() const;
    Collection collection() const;
    Protobuf *_protobuf;
    ModelItem *_item;
};

#endif // RTSYS_MISSION_MODEL_BACKEND_H
