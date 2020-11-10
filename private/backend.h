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
    google::protobuf::Message *addPoint();
    google::protobuf::Message *addRail();

  private:
    Component componentType() const;
    Collection collectionType() const;
    Component parentComponentType() const;

    google::protobuf::Message *_protobuf;
    MissionItem *_item;
};

#endif // RTSYS_MISSION_BACKEND_H
