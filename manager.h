#ifndef RTSYS_MISSION_MANAGER_H
#define RTSYS_MISSION_MANAGER_H

// ===
// === Include
// ============================================================================ //

#include "private/model.h"

// ===
// === Define
// ============================================================================ //

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
    void addMission(const QModelIndex &parent);
    void deleteS(const QModelIndex &index);

  private:
    MissionModel _model;
};

#endif // RTSYS_MISSION_MANAGER_H
