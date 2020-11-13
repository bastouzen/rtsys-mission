#ifndef RTSYS_MISSION_MANAGER_H
#define RTSYS_MISSION_MANAGER_H

// ===
// === Include
// ============================================================================ //

#include "mission/model.h"
#include "protobuf/mission.pb.h"

// ===
// === Class
// ============================================================================ //

class MissionManager : public QObject
{
    Q_OBJECT

  public:
    explicit MissionManager(QObject *parent = nullptr);
    ~MissionManager();

    MissionModel *model() { return &_model; }

    // Standard interface
    void loadMission(const pb::mission::Mission &mission);
    void newMission();
    void removeMission();

    // Interface Tree view
    void removeIndex(const QModelIndex &index);
    void addCollectionIndex(const QModelIndex &parent);
    void addPointIndex(const QModelIndex &parent);
    void addRailIndex(const QModelIndex &parent);
    void addSegmentIndex(const QModelIndex &parent);

  private:
    pb::mission::Mission _mission;
    MissionModel _model;
};

#endif // RTSYS_MISSION_MANAGER_H
