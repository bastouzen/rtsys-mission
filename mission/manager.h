#ifndef RTSYS_MISSION_MANAGER_H
#define RTSYS_MISSION_MANAGER_H

// ===
// === Include
// ============================================================================ //

#include "mission/item.h"
#include "mission/model.h"
#include "protobuf/mission.pb.h"

// ===
// === Class
// ============================================================================ //

class MissionManager : public QObject
{
    Q_OBJECT

  public:
    static pb::mission::Mission getMissionTemplate();

    explicit MissionManager(QObject *parent = nullptr);
    ~MissionManager();

    MissionModel *model() { return &_model; }

    // Standard interface
    void removeMission();
    void newMission();
    void loadMission(const pb::mission::Mission &mission);
    void saveMissionAs(const QString &filename);
    void openMission(const QString &filename);
    void saveMission();

    // Interface Tree view
    void removeIndex(const QModelIndex &index);
    void addIndexCollection(const QModelIndex &parent);
    void addIndexPoint(const QModelIndex &parent);
    void addIndexRail(const QModelIndex &parent);
    void addIndexSegment(const QModelIndex &parent);

  signals:
    void loadMissionDone();

  private:
    void addIndexFromFlag(const QModelIndex &parent, int flag);
    pb::mission::Mission _mission;
    MissionModel _model;
    QString _current_mission_filename;
};

#define AddIndexFlag(n)                                                                                                \
    inline void MissionManager::addIndex##n(const QModelIndex &parent) { addIndexFromFlag(parent, MissionItem::k##n); }

AddIndexFlag(Collection); // Adds a collection under the specified parent index.
AddIndexFlag(Point);      // Adds a point under the specified parent index.
AddIndexFlag(Rail);       // Adds a rail under the specified parent index.
AddIndexFlag(Segment);    // Adds a segment under the specified parent index.

#endif // RTSYS_MISSION_MANAGER_H
