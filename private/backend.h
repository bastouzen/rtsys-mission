#ifndef RTSYS_MISSION_BACKEND_H
#define RTSYS_MISSION_BACKEND_H

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

class MissionItem;

// ===
// === Class
// ============================================================================ //

// This defines the backend of the mission.
class MissionBackend
{
  public:
    enum Component { kMission, kCollection, kPoint, kRail, kSegment, kNoComponent };
    enum Collection { kScenario, kRoute, kFamily };
    enum Action { kDelete, kAddPoint, kAddRail, kAddSegment, kAddCollection };

  public:
    explicit MissionBackend(google::protobuf::Message *protobuf = nullptr, MissionItem *item = nullptr);
    ~MissionBackend();

    QVariant icon() const;
    unsigned int maskEnableAction() const;
    bool hasEnableAction(const Action action) const { return hasEnableAction(action, maskEnableAction()); }
    bool hasEnableAction(const Action action, const unsigned int mask) const { return (mask >> action) & 1; }

    void remove(const int row);
    void clear();

    template <class T>
    google::protobuf::Message *addElement(const QString &name = QString("Element"));
    google::protobuf::Message *addCollection(const QString &name = QString("Collection"));

  private:
    Component componentType() const;
    Collection collectionType() const;
    Component parentComponentType() const;

    google::protobuf::Message *_protobuf;
    MissionItem *_item;
};

template <class T>
google::protobuf::Message *MissionBackend::addElement(const QString &name)
{
    T *element = nullptr;
    const auto &component_type = componentType();
    if (component_type == MissionBackend::kMission) {
        element = static_cast<pb::mission::Mission *>(_protobuf)->add_components()->mutable_element()->mutable_point();
    } else if (component_type == MissionBackend::kCollection) {
        element = static_cast<pb::mission::Mission::Collection *>(_protobuf)->add_elements()->mutable_point();
    } else {
        qWarning() << "MissionBackend" << __func__ << "adding" << name << " not implemented for component type"
                   << component_type;
        return element;
    }

    element->set_name(QString("%1 %2").arg(name).arg(_item->childCount()).toStdString());
    return element;
}

#endif // RTSYS_MISSION_BACKEND_H
