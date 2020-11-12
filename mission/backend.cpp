// ===
// === Include
// ============================================================================ //

#include "mission/backend.h"
#include "mission/item.h"
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
// === Function
// ============================================================================ //

// Returns the component type of the underlying protobuf message. The component
// type is figured out by looking at the protobuf message descriptor and then
// check for name matching.
MissionBackend::Component MissionBackend::componentType(google::protobuf::Message *protobuf)
{
    if (!protobuf) return Component::kNoComponent;

    const auto &name = protobuf->GetDescriptor()->name();
    if (name == ComponentTypeMission) return Component::kMission;
    if (name == ComponentTypeCollection) return Component::kCollection;
    if (name == ComponentTypePoint) return Component::kPoint;
    if (name == ComponentTypeRail) return Component::kRail;
    if (name == ComponentTypeSegment) return Component::kSegment;
    return Component::kNoComponent;
}

// Returns the data extrated from the specified protobuf message.
QVector<QVariant> MissionBackend::data(google::protobuf::Message *protobuf)
{
    if (!protobuf) return {QVariant(), QVariant()};

    const auto &name = protobuf->GetDescriptor()->name();
    if (name == ComponentTypeMission)
        return {QString::fromStdString(name),
                QString::fromStdString(static_cast<pb::mission::Mission *>(protobuf)->name())};
    if (name == ComponentTypeCollection)
        return {QString::fromStdString(name),
                QString::fromStdString(static_cast<pb::mission::Mission::Collection *>(protobuf)->name())};
    if (name == ComponentTypePoint)
        return {QString::fromStdString(name),
                QString::fromStdString(static_cast<pb::mission::Mission::Element::Point *>(protobuf)->name())};
    if (name == ComponentTypeRail)
        return {QString::fromStdString(name),
                QString::fromStdString(static_cast<pb::mission::Mission::Element::Rail *>(protobuf)->name())};
    if (name == ComponentTypeSegment)
        return {QString::fromStdString(name),
                QString::fromStdString(static_cast<pb::mission::Mission::Element::Segment *>(protobuf)->name())};
    return {QVariant(), QVariant()};
}

// Remove an index of a google protobuf repeated field. As the current repeated
// field doesn't provide any removeAt function we use the SwapElements and the
// RemoveLast function.
template <class T>
void RepeatedFieldRemoveAt(const int row, google::protobuf::RepeatedPtrField<T> *repeated)
{
    for (int i = row; i < repeated->size() - 1; i++) {
        repeated->SwapElements(i, i + 1);
    }
    repeated->RemoveLast();
}

// ===
// === Class
// ============================================================================ //

MissionBackend::MissionBackend(google::protobuf::Message *protobuf, MissionItem *item)
    : _protobuf(protobuf)
    , _item(item)
{
}

MissionBackend::~MissionBackend() {}

// Returns the representing icon of the underlying protobuf message. The icon
// is figured out by looking at the componenet and collection type.
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

// Returns the component type of the underlying protobuf message. The component
// type is figured out by looking at the protobuf message descriptor and then
// check for name matching.
MissionBackend::Component MissionBackend::componentType() const
{
    return componentType(_protobuf);
}

// Returns the item parent component type of parent the underlying protobuf
// message.
MissionBackend::Component MissionBackend::parentComponentType() const
{
    if (!_item || !_item->parent()) return Component::kNoComponent;

    return _item->parent()->backend().componentType();
}

// Returns the collection type of the underlying protobuf message. The collection
// type is figured out by looking at the item children component type.
//  - A Route is a collection of Point.
//  - A Family is a collection of Rail.
MissionBackend::Collection MissionBackend::collectionType() const
{
    if (!_item) return Collection::kScenario;

    auto is_route = true;
    auto is_family = true;
    for (auto *child_item : _item->_childs) {
        const auto &component_type = child_item->backend().componentType();
        is_route &= component_type == Component::kPoint;
        is_family &= component_type == Component::kRail;
    }
    if (is_route && !is_family) return Collection::kRoute;
    if (is_family && !is_route) return Collection::kFamily;
    return Collection::kScenario;
}

// Returns the mask of the enabled action depending on the component type of
// the underlying protobuf message.
unsigned int MissionBackend::maskAction() const
{
    const auto &component_type = componentType();

    if (component_type == MissionBackend::kMission) {
        return (1 << kDelete) | (1 << kAddPoint) | (1 << kAddRail) | (1 << kAddSegment) | (1 << kAddCollection);

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

// Remove the children underlying protobuf message specified by the row.
void MissionBackend::remove(const int row)
{
    const auto &component_type = componentType();

    if (component_type == MissionBackend::kMission) {
        RepeatedFieldRemoveAt(row, static_cast<pb::mission::Mission *>(_protobuf)->mutable_components());

    } else if (component_type == MissionBackend::kCollection) {
        RepeatedFieldRemoveAt(row, static_cast<pb::mission::Mission::Collection *>(_protobuf)->mutable_elements());

    } else if (component_type == MissionBackend::kNoComponent) {
        // In this case we want to remove a top-level item, it means that the
        // current backend is the one for the root item. In order to remove the
        // row-element we first retrieve the child item specified by the row
        // and the we clear the underlying protobuf message.
        if (_item->childCount() >= row) {
            _item->child(row)->backend().clear();
        }

    } else {
        qWarning() << "MissionBackend " << __func__ << "removing operation not implemented";
    }
}

// Clears the underlying protobuf message.
void MissionBackend::clear()
{
    if (_protobuf) _protobuf->Clear();
}

/*
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */

//// Adds a point protobuf message under the underlying protobuf message.
//// Depending on the component type, the point is added either into the
//// component or the collection.
// google::protobuf::Message *MissionBackend::addPoint()
//{
//    const auto &component_type = componentType();

//    pb::mission::Mission::Element::Point *element = nullptr;
//    AddElement(point);

//    element->set_name(QString("Point %1").arg(_item->childCount()).toStdString());
//    return element;
//}

//// Adds a rail protobuf message under the underlying protobuf message.
//// Depending on the component type, the rail is added either into the
//// component or the collection.
// google::protobuf::Message *MissionBackend::addRail()
//{
//    const auto &component_type = componentType();

//    pb::mission::Mission::Element::Rail *element = nullptr;
//    AddElement(rail);

//    element->set_name(QString("Rail %1").arg(_item->childCount()).toStdString());
//    element->mutable_p0()->set_name("JA");
//    element->mutable_p0()->set_name("JB");
//    return element;
//}

//// Adds a segment protobuf message under the underlying protobuf message.
//// Depending on the component type, the segment is added either into the
//// component or the collection.
// google::protobuf::Message *MissionBackend::addSegment()
//{
//    const auto &component_type = componentType();

//    pb::mission::Mission::Element::Segment *element = nullptr;
//    AddElement(segment);

//    element->set_name(QString("Segment %1").arg(_item->childCount()).toStdString());
//    element->mutable_p0()->set_name("SA");
//    element->mutable_p0()->set_name("SB");
//    return element;
//}

//// Adds a collection protobuf message under the underlying protobuf message.
// google::protobuf::Message *MissionBackend::addCollection()
//{
//    const auto &component_type = componentType();

//    pb::mission::Mission::Collection *collection = nullptr;
//    if (component_type == MissionBackend::kMission)
//        collection = static_cast<pb::mission::Mission *>(_protobuf)->add_components()->mutable_collection();
//    if (!collection) {
//        qWarning() << "MissionBackend" << __func__ << "adding collection not implemented for component type"
//                   << component_type;
//        return collection;
//    }

//    collection->set_name(QString("Collection %1").arg(_item->childCount()).toStdString());
//    return collection;
//}

//#define AddElement(name)                                                                                               \
//    if (component_type == kMission)                                                                                    \
//        element =                                                                                                      \
//            static_cast<pb::mission::Mission *>(_protobuf)->add_components()->mutable_element()->mutable_##name();     \
//    if (component_type == kCollection)                                                                                 \
//        element = static_cast<pb::mission::Mission::Collection *>(_protobuf)->add_elements()->mutable_##name();        \
//                                                                                                                       \
//    if (!element) {                                                                                                    \
//        qWarning() << "MissionBackend" << __func__ << "adding" << #name << "not implemented for component type"        \
//                   << component_type;                                                                                  \
//        return element;                                                                                                \
//    }
