// ===
// === Include
// ============================================================================ //

#include "manager.h"

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

// This creates a new mission.
void MissionManager::newMission()
{
    clearMission();

    _mission.set_name("My New Mission");
    _model.insertRow<decltype(_mission)>(0, QModelIndex(), &_mission);
}

// This clears the existing mission
void MissionManager::clearMission()
{
    _mission.Clear();
    _model.removeRow(0, QModelIndex());
}

// This adds a point into the parent index model.
void MissionManager::addPoint(const QModelIndex &parent)
{
    auto *parent_item = parent.isValid() ? _model.item(parent) : _model.root();
    auto *protobuf = static_cast<pb::mission::Mission::Element::Point *>(parent_item->backend()->addPoint());
    const auto &row = _model.rowCount(parent);
    protobuf->set_name(QString("My Point %1").arg(row).toStdString());
    _model.insertRow<pb::mission::Mission::Element::Point>(row, parent, protobuf);
}

// This removes the object specify by the index model.
void MissionManager::remove(const QModelIndex &index)
{
    qInfo() << objectName() << ":: remove" << index;
    if (index.isValid()) {
        _model.removeRow(index.row(), index.parent());
    }
}

void MissionManager::loadMission(pb::mission::Mission *mission)
{
    auto appendItem = [&](auto *msg, MissionItem *parent) {
        auto *item = new MissionItem(
            {QString::fromStdString(msg->GetDescriptor()->name()), QString::fromStdString(msg->name())}, msg, parent);
        parent->appendChild(item);
        return item;
    };

    auto appendElement = [&](pb::mission::Mission::Element *element, MissionItem *parent) {
        MissionItem *elder;
        switch (element->element_case()) {
            case pb::mission::Mission::Element::kPoint:
                appendItem(element->mutable_point(), parent);
                break;
            case pb::mission::Mission::Element::kRail:
                elder = appendItem(element->mutable_rail(), parent);
                appendItem(element->mutable_rail()->mutable_p0(), elder);
                appendItem(element->mutable_rail()->mutable_p1(), elder);
                break;
            case pb::mission::Mission::Element::kSegment:
                elder = appendItem(element->mutable_segment(), parent);
                appendItem(element->mutable_segment()->mutable_p0(), elder);
                appendItem(element->mutable_segment()->mutable_p1(), elder);
                break;
            default:
                break;
        }
    };

    auto appendCollection = [&](pb::mission::Mission::Collection *collection, MissionItem *parent) {
        auto *elder = appendItem(collection, parent);
        for (auto &element : *collection->mutable_elements()) {
            appendElement(&element, elder);
        }
    };

    auto *elder = appendItem(mission, _model.root());

    for (auto &component : *mission->mutable_components()) {
        switch (component.component_case()) {
            case pb::mission::Mission::Component::kElement:
                appendElement(component.mutable_element(), elder);
                break;
            case pb::mission::Mission::Component::kCollection:
                appendCollection(component.mutable_collection(), elder);
                break;
            default:
                break;
        }
    }
}
