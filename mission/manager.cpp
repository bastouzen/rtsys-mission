// ===
// === Include
// ============================================================================ //

#include "mission/manager.h"
#include "mission/item.h"
#include "protobuf/misc/misc_cpp.h"

#include <QFile>
#include <QLoggingCategory>

// ===
// === Define
// ============================================================================ //

Q_LOGGING_CATEGORY(LC_RMMG, "rtsys.mission.manager")

// ===
// === Function
// ============================================================================ //

// This generates a template for the mission protobuf message.
rtsys::mission::Mission MissionManager::getMissionTemplate()
{
    rtsys::mission::Mission mission;
    rtsys::mission::Collection *collection;
    rtsys::mission::Block *block;
    rtsys::mission::Device *device;
    rtsys::mission::Payload::Navigation *navigation;

    auto make_default_line = [](rtsys::mission::Block::Line *line, const rtsys::mission::Block::Line::Type type,
                                const std::string &name) {
        line->set_name(name);
        line->set_type(type);
        line->add_points()->set_name(name + "A");
        line->add_points()->set_name(name + "B");
    };

    mission.set_name("RTSys Template Mission");
    mission.set_brief("This is a simple brief about my mission. Basically we have to set the datatime");

    block = mission.add_components()->mutable_block();
    block->mutable_point()->set_name("P0");
    navigation = block->add_payloads()->mutable_navigation();
    navigation->set_depth(10.0);
    navigation->set_heading(270);
    navigation->set_velocity(3.0);

    make_default_line(mission.add_components()->mutable_block()->mutable_line(), rtsys::mission::Block::Line::LINE_RAIL,
                      "J0");

    collection = mission.add_components()->mutable_collection();
    collection->set_name("Scenario");
    collection->add_blocks()->mutable_point()->set_name("P1");
    make_default_line(collection->add_blocks()->mutable_line(), rtsys::mission::Block::Line::LINE_RAIL, "J0");
    make_default_line(collection->add_blocks()->mutable_line(), rtsys::mission::Block::Line::LINE_SEGMENT, "S0");

    collection = mission.add_components()->mutable_collection();
    collection->set_name("Route");
    for (int i = 0; i < 5; i++) {
        collection->add_blocks()->mutable_point()->set_name("R" + std::to_string(i));
    }

    make_default_line(mission.add_components()->mutable_block()->mutable_line(),
                      rtsys::mission::Block::Line::LINE_SEGMENT, "S1");

    collection = mission.add_components()->mutable_collection();
    collection->set_name("Familly");
    for (int i = 0; i < 6; i++) {
        make_default_line(collection->add_blocks()->mutable_line(), rtsys::mission::Block::Line::LINE_RAIL,
                          "J" + std::to_string(i));
    }

    device = mission.add_components()->mutable_device();
    device->set_name("My Device");
    device->set_type(rtsys::mission::Device_Type_DEVICE_SURFACE);
    collection = device->add_components()->mutable_collection();
    collection->set_name("My Familly");
    for (int i = 0; i < 6; i++) {
        make_default_line(collection->add_blocks()->mutable_line(), rtsys::mission::Block::Line::LINE_RAIL,
                          "J" + std::to_string(i));
    }

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
    addIndexDevice(_model.index(0, 0, QModelIndex()));
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
    qCWarning(LC_RMMG) << "fail removing mission, root item children" << _model.root()->countChild();
}

// This creates a new mission.
void MissionManager::newMission()
{
    removeMission();
    Q_ASSERT(_model.rowCount() == 0); // assert root item has no children

    auto parent = QModelIndex();
    auto row = _model.rowCount(parent);
    _model.insertRow(row, parent);
    _model.setData(_model.index(row, 0, parent), QVariant::fromValue(MissionItem::wrap(&_mission)),
                   Qt::UserRoleWrapper);
    _model.setData(_model.index(row, 0, parent), MissionItem::Features::Int(MissionItem::kMission),
                   Qt::UserRoleFeature);
}

// This loads a mission.
void MissionManager::loadMission(const rtsys::mission::Mission &mission)
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
        qCInfo(LC_RMMG) << "save the mission into" << json_file.fileName();
    } else {
        qCWarning(LC_RMMG) << "fail saving the mission, can't open file" << json_file.fileName();
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
        rtsys::mission::Mission mission;
        rtsys::protobuf::misc::parseFromJson(&mission, json_file.readAll().toStdString());
        loadMission(mission);
        _current_mission_filename = filename;
        qCInfo(LC_RMMG) << "open the mission from" << json_file.fileName();
    } else {
        qCWarning(LC_RMMG) << "fail opening mission, can't open file" << json_file.fileName();
    }
}

// This saves the mission into the current mission filename.
void MissionManager::saveMission()
{
    if (canSaveMission()) {
        saveMissionAs(_current_mission_filename);
        return;
    }
    qCWarning(LC_RMMG) << "fail saving mission, missing reference" << _current_mission_filename;
}

// Remove the specified model index from the internal model mission.
void MissionManager::removeIndex(const QModelIndex &index)
{
    if (index.isValid()) _model.removeRow(index.row(), index.parent());
}

// TODO
void MissionManager::swapIndex(const QModelIndex &index)
{
    _model.swapIndex(index);
}

// Adds a flag identifier under the specified parent index.
void MissionManager::addIndexFromFlag(const QModelIndex &parent, MissionItem::Features feature)
{
    if (!parent.isValid()) return;

    // Set kLine if kRail or kSegment has been set.
    if (feature & (MissionItem::kRail | MissionItem::kSegment)) {
        feature |= MissionItem::kLine;
    }

    auto row = _model.rowCount(parent);
    _model.insertRow(row, parent);
    _model.setData(_model.index(row, 0, parent), MissionItem::Features::Int(feature), Qt::UserRoleFeature);
}
