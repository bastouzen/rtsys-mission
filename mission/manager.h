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
    static pb::mission::Mission getMissionTemplate();

    explicit MissionManager(QObject *parent = nullptr);
    ~MissionManager();

    MissionModel *model() { return &_model; }

    // Standard interface
    void loadMission(const pb::mission::Mission &mission);
    void newMission();
    void removeMission();
    void saveMissionAs(const QString &filename);
    void openMission(const QString &filename);
    void saveMission();

    // Interface Tree view
    void removeIndex(const QModelIndex &index);
    void addCollectionIndex(const QModelIndex &parent);
    void addPointIndex(const QModelIndex &parent);
    void addRailIndex(const QModelIndex &parent);
    void addSegmentIndex(const QModelIndex &parent);

  signals:
    void loadMissionDone();

  private:
    pb::mission::Mission _mission;
    MissionModel _model;
    QString _current_mission_filename;
};

#endif // RTSYS_MISSION_MANAGER_H
