// ===
// === Include
// ============================================================================ //

#include "manager.h"
#include "private/model_misc.h"
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
    //    addPoint(_model.index(0, 0, QModelIndex()));
    //    addPoint(_model.index(0, 0, QModelIndex()));
    //    addPoint(_model.index(0, 0, QModelIndex()));
    //    addRail(_model.index(0, 0, QModelIndex()));
}

MissionManager::~MissionManager() {}

void MissionManager::newMission()
{
    removeMission();
    //_model.insertRow<decltype(_mission)>(0, QModelIndex(), &_mission);
}

//// This clears the existing mission
// void MissionManager::clearMission()
//{
//    _model.removeRow(0, QModelIndex());
//}

//// Remove the index of the model specified by the given index. First we check if
//// the index is valid and then use its parent index and its row for removing it
// void MissionManager::remove(const QModelIndex &index)
//{
//    if (!index.isValid()) return;

//    qDebug() << index << index.isValid() << index.parent() << index.parent().isValid();

//    if (index.parent().isValid())
//        _model.removeRow(index.row(), index.parent());
//    else
//        clearMission();
//}

//// Adds a point under the specified parent index. This check if the parent is
//// valid and if the "addPoint" action is enabled for the specified parent index.
// void MissionManager::addPoint(const QModelIndex &parent)
//{
//    _model.addPoint(parent);
//}

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

void MissionManager::loadMission(const pb::mission::Mission &mission)
{
    removeMission();

    //    auto appendItem = [&](auto *msg, MissionItem *parent) {
    //        auto *item = new MissionItem(
    //            {QString::fromStdString(msg->GetDescriptor()->name()), QString::fromStdString(msg->name())}, msg,
    //            parent);
    //        // parent->appendChild(item);
    //        return item;
    //    };

    //    auto appendElement = [&](pb::mission::Mission::Element *element, MissionItem *parent) {
    //        MissionItem *elder;
    //        switch (element->element_case()) {
    //            case pb::mission::Mission::Element::kPoint:
    //                appendItem(element->mutable_point(), parent);
    //                break;
    //            case pb::mission::Mission::Element::kRail:
    //                elder = appendItem(element->mutable_rail(), parent);
    //                appendItem(element->mutable_rail()->mutable_p0(), elder);
    //                appendItem(element->mutable_rail()->mutable_p1(), elder);
    //                break;
    //            case pb::mission::Mission::Element::kSegment:
    //                elder = appendItem(element->mutable_segment(), parent);
    //                appendItem(element->mutable_segment()->mutable_p0(), elder);
    //                appendItem(element->mutable_segment()->mutable_p1(), elder);
    //                break;
    //            default:
    //                break;
    //        }
    //    };

    //    auto appendCollection = [&](pb::mission::Mission::Collection *collection, MissionItem *parent) {
    //        auto *elder = appendItem(collection, parent);
    //        for (auto &element : *collection->mutable_elements()) {
    //            appendElement(&element, elder);
    //        }
    //    };

    _mission.CopyFrom(mission);
    _model.appendRow(QModelIndex(), &_mission);

    //    for (auto &component : *_mission.mutable_components()) {
    //        switch (component.component_case()) {
    //            case pb::mission::Mission::Component::kElement:
    //                _model.appendRow(_model.index(0, 0, QModelIndex()), component.mutable_element());
    //                break;
    //            case pb::mission::Mission::Component::kCollection:
    //                _model.appendRow(_model.index(0, 0, QModelIndex()), component.mutable_collection());
    //                break;
    //            default:
    //                break;
    //        }
    //    }
}
