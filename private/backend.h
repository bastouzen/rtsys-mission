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

  public:
    explicit MissionBackend(google::protobuf::Message *protobuf = nullptr, MissionItem *item = nullptr);
    ~MissionBackend();

    QVariant icon() const;
    Component componentType() const;
    Collection collectionType() const;

  private:
    Component parentComponentType() const;

    google::protobuf::Message *_protobuf;
    MissionItem *_item;
};

#endif // RTSYS_MISSION_BACKEND_H
