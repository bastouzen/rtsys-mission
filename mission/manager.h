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
    void removeMission() {}

    // Interface Tree view
    void remove(const QModelIndex &index) {}
    void addPoint(const QModelIndex &parent) {}
    void addRail(const QModelIndex &parent) {}

  private:
    pb::mission::Mission _mission;
    MissionModel _model;

    //    void newMission();
    //    void clearMission();
};

#endif // RTSYS_MISSION_MANAGER_H
