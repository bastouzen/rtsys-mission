// ===
// === Include
// ============================================================================ //

#include "mission/manager.h"
#include "mission/item.h"
#include "mission/misc.h"
#include <QDebug>

// ===
// === Define
// ============================================================================ //

#define CLASSNAME "MissionManager ::"

// ===
// === Class
// ============================================================================ //

MissionManager::MissionManager(QObject *parent)
    : QObject(parent)
{
    newMission();
}

MissionManager::~MissionManager() {}

// This loads a mission. First we remove the internal mission protobuf then
// copy the specified mission protobuf into the internal mission protobuf and
// finaly update the internal mission model.
void MissionManager::loadMission(const pb::mission::Mission &mission)
{
    removeMission();
    _mission.CopyFrom(mission);
    _model.appendRow(QModelIndex(), &_mission);
}

// This creates a new mission. First we remove the internal mission protobuf
// then set the internal mission protobuf name and finaly update the internal
// mission model.
void MissionManager::newMission()
{
    removeMission();
    _mission.set_name(QString(tr("My New Mission")).toStdString());
    _model.appendRow(QModelIndex(), &_mission);
}

// This removes the mission. Here we suppose that the root item has only one
// child which is the internal mission item.
void MissionManager::removeMission()
{
    const auto root_child_count = _model.root()->childCount();

    if (!root_child_count) return;

    if (_model.root()->childCount() == 1) {
        auto mission = _model.root()->child(0);
        removeIndex(_model.index(mission));
        return;
    }
    qWarning() << CLASSNAME << "[Warning] fail removing mission, root item children" << _model.root()->childCount();
}

// Remove the specified model index from the internal model mission.
void MissionManager::removeIndex(const QModelIndex &index)
{
    if (index.isValid()) _model.removeRow(index.row(), index.parent());
}

// Adds a collection under the specified parent index.
void MissionManager::addCollectionIndex(const QModelIndex &index)
{
    if (index.isValid()) _model.appendRow(index, ModelBacken::kAddCollection);
}

// Adds a point under the specified parent index.
void MissionManager::addPointIndex(const QModelIndex &index)
{
    if (index.isValid()) _model.appendRow(index, ModelBacken::kAddPoint);
}

// Adds a rail under the specified parent index.
void MissionManager::addRailIndex(const QModelIndex &index)
{
    if (index.isValid()) _model.appendRow(index, ModelBacken::kAddRail);
}

// Adds a segment under the specified parent index.
void MissionManager::addSegmentIndex(const QModelIndex &index)
{
    if (index.isValid()) _model.appendRow(index, ModelBacken::kAddSegment);
}
