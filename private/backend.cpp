// ===
// === Include
// ============================================================================ //

#include "private/backend.h"
#include "private/model.h"
#include "protobuf/mission.pb.h"

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

MissionBackend::Component MissionBackend::componentType() const
{
    if (!_protobuf) return Component::kMission;

    if (_protobuf->GetDescriptor()->name() == ComponentTypeMission) return Component::kMission;
    if (_protobuf->GetDescriptor()->name() == ComponentTypeCollection) return Component::kCollection;
    if (_protobuf->GetDescriptor()->name() == ComponentTypePoint) return Component::kPoint;
    if (_protobuf->GetDescriptor()->name() == ComponentTypeRail) return Component::kRail;
    if (_protobuf->GetDescriptor()->name() == ComponentTypeSegment) return Component::kSegment;
    assert(false);
}

MissionBackend::Collection MissionBackend::collectionType() const
{
    if (!_item) return Collection::kScenario;

    auto is_route = true;
    auto is_family = true;
    for (auto *child : _item->childs()) {
        const auto &component_type = child->backend()->componentType();
        is_route &= component_type == Component::kPoint;
        is_family &= component_type == Component::kRail;
    }
    if (is_route) return Collection::kRoute;
    if (is_family) return Collection::kFamily;
    return Collection::kScenario;
}

MissionBackend::Component MissionBackend::parentComponentType() const
{
    return _item->parent()->backend()->componentType();
}
