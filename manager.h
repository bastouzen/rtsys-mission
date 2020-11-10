#ifndef RTSYS_MISSION_MANAGER_H
#define RTSYS_MISSION_MANAGER_H

// ===
// === Include
// ============================================================================ //

#include "private/model.h"
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
    void loadMission(pb::mission::Mission *mission);

    void remove(const QModelIndex &index);
    void addPoint(const QModelIndex &parent);
    void addRail(const QModelIndex &parent);

  private:
    pb::mission::Mission _mission;
    void newMission();
    void clearMission();
    MissionModel _model;
};

#endif // RTSYS_MISSION_MANAGER_H
