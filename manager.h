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

    void newMission();
    void clearMission();
    void addPoint(const QModelIndex &parent);
    void remove(const QModelIndex &index);

    pb::mission::Mission _mission;
  private:
    MissionModel _model;
};

#endif // RTSYS_MISSION_MANAGER_H
