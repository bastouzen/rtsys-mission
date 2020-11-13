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
    enum Action { kDelete, kAddCollection, kAddPoint, kAddRail, kAddSegment };

    static QVector<QVariant> data(google::protobuf::Message *protobuf);
    static Component componentType(google::protobuf::Message *protobuf);

  public:
    explicit MissionBackend(google::protobuf::Message *protobuf = nullptr, MissionItem *item = nullptr);
    ~MissionBackend();

    QVariant icon() const;
    unsigned int maskAction() const;
    bool hasAction(const Action action) const { return hasAction(action, maskAction()); }
    bool hasAction(const Action action, const unsigned int mask) const { return (mask >> action) & 1; }
    void remove(const int row);
    void clear();
    google::protobuf::Message *append(const Action action);

  private:
    Component parentComponentType() const;
    Component componentType() const;
    Collection collectionType() const;
    google::protobuf::Message *_protobuf;
    MissionItem *_item;
};

#endif // RTSYS_MISSION_MODEL_BACKEND_H
