// ===
// === Include
// ============================================================================ //

#include "mission/manager.h"
#include "mission/item.h"
#include "protobuf/misc/misc_cpp.h"

#include <QDebug>
#include <QFile>

// ===
// === Define
// ============================================================================ //

#define CLASSNAME "MissionManager ::"

// ===
// === Function
// ============================================================================ //

// This generates a template for the mission protobuf message.
pb::mission::Mission MissionManager::getMissionTemplate()
{
    pb::mission::Mission mission;
    pb::mission::Mission::Component *component;
    pb::mission::Mission::Element *element;

    mission.set_name("RTSys Template Mission");

    component = mission.add_components();
    component->mutable_element()->mutable_point()->set_name("P0");

    component = mission.add_components();
    component->mutable_collection()->set_name("Scenario");

    element = component->mutable_collection()->add_elements();
    element->mutable_point()->set_name("P1");

    element = component->mutable_collection()->add_elements();
    element->mutable_rail()->set_name("R0");
    element->mutable_rail()->mutable_p0()->set_name("RA");
    element->mutable_rail()->mutable_p1()->set_name("RB");

    element = component->mutable_collection()->add_elements();
    element->mutable_segment()->set_name("S1");
    element->mutable_segment()->mutable_p0()->set_name("SA");
    element->mutable_segment()->mutable_p1()->set_name("SB");

    component = mission.add_components();
    component->mutable_element()->mutable_rail()->set_name("R0");
    component->mutable_element()->mutable_rail()->mutable_p0()->set_name("RA");
    component->mutable_element()->mutable_rail()->mutable_p1()->set_name("RB");

    component = mission.add_components();
    component->mutable_collection()->set_name("Route");
    element = component->mutable_collection()->add_elements();
    element->mutable_point()->set_name("R0");
    element = component->mutable_collection()->add_elements();
    element->mutable_point()->set_name("R1");
    element = component->mutable_collection()->add_elements();
    element->mutable_point()->set_name("R2");
    element = component->mutable_collection()->add_elements();
    element->mutable_point()->set_name("R3");
    element = component->mutable_collection()->add_elements();
    element->mutable_point()->set_name("R4");

    component = mission.add_components();
    component->mutable_element()->mutable_segment()->set_name("S0");
    component->mutable_element()->mutable_segment()->mutable_p0()->set_name("SA");
    component->mutable_element()->mutable_segment()->mutable_p1()->set_name("SB");

    component = mission.add_components();
    component->mutable_collection()->set_name("Family");
    element = component->mutable_collection()->add_elements();
    element->mutable_rail()->set_name("J1");
    element->mutable_rail()->mutable_p0()->set_name("J1A");
    element->mutable_rail()->mutable_p1()->set_name("J1B");
    element = component->mutable_collection()->add_elements();
    element->mutable_rail()->set_name("J2");
    element->mutable_rail()->mutable_p0()->set_name("J2A");
    element->mutable_rail()->mutable_p1()->set_name("J2B");
    element = component->mutable_collection()->add_elements();
    element->mutable_rail()->set_name("J3");
    element->mutable_rail()->mutable_p0()->set_name("J3A");
    element->mutable_rail()->mutable_p1()->set_name("J3B");
    element = component->mutable_collection()->add_elements();
    element->mutable_rail()->set_name("J4");
    element->mutable_rail()->mutable_p0()->set_name("J4A");
    element->mutable_rail()->mutable_p1()->set_name("J4B");
    element = component->mutable_collection()->add_elements();
    element->mutable_rail()->set_name("J5");
    element->mutable_rail()->mutable_p0()->set_name("J5A");
    element->mutable_rail()->mutable_p1()->set_name("J5B");

    return mission;
}

// ===
// === Class
// ============================================================================ //

MissionManager::MissionManager(QObject *parent)
    : QObject(parent)
{
    newMission();
    addIndexPoint(_model.index(0, 0, QModelIndex()));
    addIndexRail(_model.index(0, 0, QModelIndex()));
    addIndexSegment(_model.index(0, 0, QModelIndex()));
    addIndexCollection(_model.index(0, 0, QModelIndex()));
}

MissionManager::~MissionManager() {}

// This removes the mission. Here we suppose that the root item has only one
// child which is the internal mission item.
void MissionManager::removeMission()
{
    const auto &root_child_count = _model.root()->countChild();

    if (!root_child_count) return;
    if (root_child_count == 1) {
        removeIndex(_model.index(_model.root()->child(0)));
        return;
    }
    qWarning() << CLASSNAME << "[Warning] fail removing mission, root item children" << _model.root()->countChild();
}

// This creates a new mission.
void MissionManager::newMission()
{
    removeMission();
    Q_ASSERT(_model.rowCount() == 0); // assert root item has no children

    auto parent = QModelIndex();
    auto row = _model.rowCount(parent);
    _model.insertRow(row, parent);
    _model.item(_model.index(row, 0, parent))->setProtobuf(&_mission);
    _model.setData(_model.index(row, 0, parent), QVariant::fromValue(MissionItem::kMission), Qt::UserRoleFlag);
}

// This loads a mission.
void MissionManager::loadMission(const pb::mission::Mission &mission)
{
    newMission();
    auto index = _model.index(0, 0, QModelIndex()); // index of the root children for the mission
    _model.setData(index, QVariant::fromValue(MissionItem::pack(mission)), Qt::UserRolePack);
    emit loadMissionDone();
}

// This saves the mission into the specified filename path. This is using the
// protobuf capability in order to save the internal mission protobuf message
// in json format.
void MissionManager::saveMissionAs(const QString &filename)
{
    if (filename.isEmpty()) return;

    QFile json_file(filename.endsWith(".json") ? filename : filename + ".json");
    if (json_file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        std::string json_stream;
        rtsys::protobuf::misc::serializeToJson(_mission, &json_stream);
        json_file.write(json_stream.data());
        _current_mission_filename = filename;
        qInfo() << CLASSNAME << "[Info] save the mission into" << json_file.fileName();
    } else {
        qWarning() << CLASSNAME << "[Warning] fail saving the mission, can't open file" << json_file.fileName();
    }
}

// This opens the mission from the specified filename path. This is using the
// protobuf capability in order to retrieve the internal mission protobuf message
// from the json format.
void MissionManager::openMission(const QString &filename)
{
    if (filename.isEmpty()) return;

    QFile json_file(filename);
    if (json_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        pb::mission::Mission mission;
        rtsys::protobuf::misc::parseFromJson(&mission, json_file.readAll().toStdString());
        loadMission(mission);
        _current_mission_filename = filename;
        qInfo() << CLASSNAME << "[Info] open the mission from" << json_file.fileName();
    } else {
        qWarning() << CLASSNAME << "[Warning] fail opening mission, can't open file" << json_file.fileName();
    }
}

// This saves the mission into the current mission filename.
void MissionManager::saveMission()
{
    if (_current_mission_filename.isEmpty()) {
        qWarning() << CLASSNAME << "[Warning] fail saving mission, missing reference" << _current_mission_filename;
        return;
    }
    saveMissionAs(_current_mission_filename);
}

// Remove the specified model index from the internal model mission.
void MissionManager::removeIndex(const QModelIndex &index)
{
    if (index.isValid()) _model.removeRow(index.row(), index.parent());
}

// Adds a flag identifier under the specified parent index.
void MissionManager::addIndexFromFlag(const QModelIndex &parent, int flag)
{
    if (!parent.isValid()) return;

    auto row = _model.rowCount(parent);
    _model.insertRow(row, parent);
    _model.setData(_model.index(row, 0, parent), QVariant::fromValue(flag), Qt::UserRoleFlag);
}
