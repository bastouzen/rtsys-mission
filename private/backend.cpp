// ===
// === Include
// ============================================================================ //

#include "private/backend.h"
#include "private/model.h"
#include "private/model_misc.h"
#include "protobuf/mission.pb.h"

#include <QDebug>
#include <QIcon>

// ===
// === Define
// ============================================================================ //

const auto ResourceIconMission = QStringLiteral(":/resource/mission.svg");
const auto ResourceIconCollection = QStringLiteral(":/resource/collection.svg");
const auto ResourceIconRoute = QStringLiteral(":/resource/route.svg");
const auto ResourceIconFamily = QStringLiteral(":/resource/family.svg");
const auto ResourceIconRail = QStringLiteral(":/resource/rail.svg");
const auto ResourceIconSegment = QStringLiteral(":/resource/segment.svg");
const auto ResourceIconPoint = QStringLiteral(":/resource/point.png");
const auto ResourceIconRailPoint = QStringLiteral(":/resource/rail.png");

const auto ComponentTypeMission = pb::mission::Mission::descriptor() -> name();
const auto ComponentTypeCollection = pb::mission::Mission::Collection::descriptor() -> name();
const auto ComponentTypePoint = pb::mission::Mission::Element::Point::descriptor() -> name();
const auto ComponentTypeRail = pb::mission::Mission::Element::Rail::descriptor() -> name();
const auto ComponentTypeSegment = pb::mission::Mission::Element::Segment::descriptor() -> name();

// ===
// === Class
// ============================================================================ //

MissionBackend::MissionBackend(google::protobuf::Message *protobuf, MissionItem *item)
    : _protobuf(protobuf)
    , _item(item)
{
}

MissionBackend::~MissionBackend() {}

// Returns the component type of the underlying protobuf message.
// The component type is figured out by looking at the protobuf message
// descriptor and then check for name matching.
MissionBackend::Component MissionBackend::componentType() const
{
    if (!_protobuf) return Component::kNoComponent;

    const auto &name = _protobuf->GetDescriptor()->name();
    if (name == ComponentTypeMission) return Component::kMission;
    if (name == ComponentTypeCollection) return Component::kCollection;
    if (name == ComponentTypePoint) return Component::kPoint;
    if (name == ComponentTypeRail) return Component::kRail;
    if (name == ComponentTypeSegment) return Component::kSegment;
    return Component::kNoComponent;
}

// Returns the item parent component type of parent the underlying protobuf
// message.
MissionBackend::Component MissionBackend::parentComponentType() const
{
    if (!_item || !_item->parent()) return Component::kNoComponent;

    return _item->parent()->backend().componentType();
}

// Returns the collection type of the underlying protobuf message.
// The collection type is figured out by looking at the item children
// component type.
//  - A Route is a collection of Point.
//  - A Family is a collection of Rail.
MissionBackend::Collection MissionBackend::collectionType() const
{
    if (!_item) return Collection::kScenario;

    auto is_route = true;
    auto is_family = true;
    for (auto *child_item : _item->childs()) {
        const auto &component_type = child_item->backend().componentType();
        is_route &= component_type == Component::kPoint;
        is_family &= component_type == Component::kRail;
    }
    if (is_route) return Collection::kRoute;
    if (is_family) return Collection::kFamily;
    return Collection::kScenario;
}

// Returns the representing icon of the underlying protobuf message.
// The icon is figured out by looking at the componenet and collection type.
QVariant MissionBackend::icon() const
{
    const auto &component_type = componentType();
    if (component_type == Component::kMission) {
        return QIcon(ResourceIconMission);
    } else if (component_type == Component::kCollection) {
        const auto &collection_type = collectionType();
        if (collection_type == Collection::kRoute) return QIcon(ResourceIconRoute);
        if (collection_type == Collection::kFamily) return QIcon(ResourceIconFamily);
        return QIcon(ResourceIconCollection);
    } else if (component_type == Component::kRail) {
        return QIcon(ResourceIconRail);
    } else if (component_type == Component::kSegment) {
        return QIcon(ResourceIconSegment);
    } else if (component_type == Component::kPoint) {
        const auto &parent_component_type = parentComponentType();
        if (parent_component_type == Component::kRail) return QIcon(ResourceIconRailPoint);
        if (parent_component_type == Component::kSegment) return QIcon(ResourceIconRailPoint);
        return QIcon(ResourceIconPoint);
    }
    return QVariant();
}

// Returns the mask of the enabled action depending on the component type of
// the underlying protobuf message.
unsigned int MissionBackend::maskEnableAction() const
{
    const auto &component_type = componentType();
    if (component_type == MissionBackend::kMission) {
        // return (1 << kDelete) | (1 << kAddPoint) | (1 << kAddRail) | (1 << kAddSegment) | (1 << kAddCollection);
        return (1 << kAddPoint) | (1 << kAddRail) | (1 << kAddSegment) | (1 << kAddCollection);
    } else if (component_type == MissionBackend::kCollection) {
        return (1 << kDelete) | (1 << kAddPoint) | (1 << kAddRail) | (1 << kAddSegment);
    } else if (component_type == MissionBackend::kRail || component_type == MissionBackend::kSegment) {
        return (1 << kDelete);
    } else if (component_type == MissionBackend::kPoint) {
        const auto &parent_component_type = parentComponentType();
        if (parent_component_type != MissionBackend::kRail && parent_component_type != MissionBackend::kSegment) {
            return (1 << kDelete);
        }
    }
    return 0;
}

// Clears the underlying protobuf message.
void MissionBackend::clear()
{
    if (!_protobuf) return;
    _protobuf->Clear();
}

// Remove the component type of the underlying protobuf message.
// Depending on the component type, we remove the row-element of the component
// type list. This is done by first swapping the elements and then removing
// the last element of the component type list.
void MissionBackend::remove(const int row)
{
    const auto &component_type = componentType();

    if (component_type == MissionBackend::kMission) {
        // Remove the row-element of the components repeated field
        auto *mission = static_cast<pb::mission::Mission *>(_protobuf);
        for (int i = row; i < mission->components_size() - 1; i++) {
            mission->mutable_components()->SwapElements(i, i + 1);
        }
        mission->mutable_components()->RemoveLast();

    } else if (component_type == MissionBackend::kCollection) {
        // Remove the row-element of the collection repeated field
        auto *collection = static_cast<pb::mission::Mission::Collection *>(_protobuf);
        for (int i = row; i < collection->elements_size() - 1; i++) {
            collection->mutable_elements()->SwapElements(i, i + 1);
        }
        collection->mutable_elements()->RemoveLast();

    } else if (component_type == MissionBackend::kNoComponent) {
        // In this case we want to remove a top-level item, it means that the
        // current backend is the one for the root item. In order to remove
        // the row-element we first retrieve the child item specified by the row
        // and the we clear the underlying protobuf message.
        if (_item->childCount() >= row) {
            _item->child(row)->backend().clear();
        }

    } else {
        qWarning() << "MissionBackend " << __func__ << "removing operation not implemented";
    }
}

// Adds a point protobuf message under the underlying protobuf message.
// Depending on the component type, the point is added either into the
// componenet or the collection.
google::protobuf::Message *MissionBackend::addPoint()
{
    pb::mission::Mission::Element::Point *point = nullptr;
    const auto &component_type = componentType();
    if (component_type == MissionBackend::kMission) {
        point = static_cast<pb::mission::Mission *>(_protobuf)->add_components()->mutable_element()->mutable_point();
    } else if (component_type == MissionBackend::kCollection) {
        point = static_cast<pb::mission::Mission::Collection *>(_protobuf)->add_elements()->mutable_point();
    } else {
        qWarning() << "MissionBackend" << __func__ << "adding point not implemented for component type"
                   << component_type;
        return point;
    }

    point->set_name(QString("Point %1").arg(_item->parent()->childCount()).toStdString());
    misc::appendItem(point, _item->parent());
    return point;
}

// Adds a rail protobuf message under the underlying protobuf message.
// Depending on the component type, the rail is added either into the
// componenet or the collection.
google::protobuf::Message *MissionBackend::addRail()
{
    const auto &component_type = componentType();
    if (component_type == MissionBackend::kMission) {
        return static_cast<pb::mission::Mission *>(_protobuf)->add_components()->mutable_element()->mutable_rail();
    } else if (component_type == MissionBackend::kCollection) {
        return static_cast<pb::mission::Mission::Collection *>(_protobuf)->add_elements()->mutable_rail();
    } else {
        qWarning() << "MissionBackend" << __func__ << "adding rail not implemented for component type"
                   << component_type;
        return nullptr;
    }
}
