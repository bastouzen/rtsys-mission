// ===
// === Include
// ============================================================================ //

#include "mission/manager.h"
#include "mission/item.h"
#include "mission/misc.h"
#include <QDebug>

// ===
// === Class
// ============================================================================ //

MissionManager::MissionManager(QObject *parent)
    : QObject(parent)
{
    setObjectName("MissionManager");

    // Initialization of the mission data structure.
    newMission();
}

MissionManager::~MissionManager() {}

void MissionManager::loadMission(const pb::mission::Mission &mission)
{
    removeMission();
    _mission.CopyFrom(mission);
    _model.appendRow(QModelIndex(), &_mission);
}

void MissionManager::newMission()
{
    removeMission();
    _mission.set_name(QString(tr("My New Mission")).toStdString());
    _model.appendRow(QModelIndex(), &_mission);
}

void MissionManager::removeMission()
{
    // Here we suppose that the root item has only one child which is the mission.
    if (_model.root()->childCount() >= 1) {
        removeIndex(_model.index(_model.root()->child(0)));
    }
}

// Remove the index of the model specified by the given index. First we check if
// the index is valid and then use its parent index and its row for removing it
void MissionManager::removeIndex(const QModelIndex &index)
{
    if (index.isValid()) _model.removeRow(index.row(), index.parent());
}

// Adds a point under the specified parent index. This check if the parent is
// valid and if the "addPoint" action is enabled for the specified parent index.
void MissionManager::addPointIndex(const QModelIndex &parent)
{
    //_model.addPoint(parent);
}

//// Adds a rail under the specified parent index. This check if the parent is
//// valid and if the "addRail" action is enabled for the specified parent index.
// void MissionManager::addRail(const QModelIndex &parent)
//{
//    //    if (!parent.isValid()) return;

//    //    const auto &parent_backend = _model.item(parent)->backend();
//    //    if (parent_backend.hasEnableAction(MissionBackend::Action::kAddRail)) {
//    //        const auto &row = _model.rowCount(parent);
//    //        auto *protobuf = static_cast<pb::mission::Mission::Element::Rail
//    //        *>(_model.item(parent)->backend().addRail()); protobuf->set_name(QString("My Rail
//    //        %1").arg(row).toStdString()); protobuf->mutable_p0()->set_name("P1");
//    //        protobuf->mutable_p1()->set_name("P2");
//    //        misc::appendRail(protobuf, _model.item(parent));
//    //    } else {
//    //        qWarning() << "MissionManager" << __func__ << "adding rail fail because action is not enabled";
//    //    }
//}
