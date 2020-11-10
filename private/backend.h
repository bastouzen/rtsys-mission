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
    enum Component { kMission, kCollection, kPoint, kRail, kSegment };
    enum Collection { kScenario, kRoute, kFamily };
    enum Action { kDelete, kAddPoint, kAddRail, kAddSegment, kAddCollection };

  public:
    explicit MissionBackend(google::protobuf::Message *protobuf = nullptr, MissionItem *item = nullptr);
    ~MissionBackend();

    QVariant icon() const;
    unsigned int action() const;

    google::protobuf::Message *protobuf() { return _protobuf; }

    void remove(const int row);
    void remove();
    google::protobuf::Message *addPoint();

  private:
    Component componentType() const;
    Collection collectionType() const;
    Component parentComponentType() const;

    google::protobuf::Message *_protobuf;
    MissionItem *_item;
};

#endif // RTSYS_MISSION_BACKEND_H
