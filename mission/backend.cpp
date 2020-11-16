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

#define CLASSNAME "ModelBackend ::"

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
const auto ComponentTypeElement = pb::mission::Mission::Element::descriptor() -> name();
const auto ComponentTypePoint = pb::mission::Mission::Element::Point::descriptor() -> name();
const auto ComponentTypeRail = pb::mission::Mission::Element::Rail::descriptor() -> name();
const auto ComponentTypeSegment = pb::mission::Mission::Element::Segment::descriptor() -> name();

// ===
// === Function
// ============================================================================ //

// Returns the component identifier specified by the underlying protobuf message.
ModelBacken::Component ModelBacken::component(const google::protobuf::Message *protobuf)
{
    if (!protobuf) return Component::kNoComponent;

    const auto &name = protobuf->GetDescriptor()->name();

    if (name == ComponentTypeMission) {
        return Component::kMission;
    } else if (name == ComponentTypeCollection) {
        return Component::kCollection;
    } else if (name == ComponentTypeElement) {
        return Component::kElement;
    } else if (name == ComponentTypePoint) {
        return Component::kPoint;
    } else if (name == ComponentTypeRail) {
        return Component::kRail;
    } else if (name == ComponentTypeSegment) {
        return Component::kSegment;
    } else {
        qWarning() << CLASSNAME << "[Warning] fail getting component identifier, missing definition" << name.data();
        return Component::kNoComponent;
    }
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

// Move the last index of a google protobuf repeated field. As the current
// repeated field doesn't provide any MoveAr function we use the SwapElements
// function.
template <class T>
void RepeatedFieldMoveLastAt(const int row, google::protobuf::RepeatedPtrField<T> *repeated)
{
    for (int i = repeated->size(); i < row; i--) {
        repeated->SwapElements(i, i - 1);
    }
}

// ===
// === Class
// ============================================================================ //

ModelBacken::ModelBacken(google::protobuf::Message *protobuf, ModelItem *item)
    : _protobuf(protobuf)
    , _item(item)
{
}

// Returns the displayed icon of the underlying protobuf message. The icon
// is figured out by looking at the componenet and collection type.
QVariant ModelBacken::icon() const
{
    const auto &component_id = component();
    if (component_id == Component::kMission) {
        return QIcon(ResourceIconMission);
    } else if (component_id == Component::kCollection) {
        const auto &collection_type = collection();
        if (collection_type == Collection::kRoute) {
            return QIcon(ResourceIconRoute);
        } else if (collection_type == Collection::kFamily) {
            return QIcon(ResourceIconFamily);
        } else {
            return QIcon(ResourceIconCollection);
        }
    } else if (component_id == Component::kRail) {
        return QIcon(ResourceIconRail);
    } else if (component_id == Component::kSegment) {
        return QIcon(ResourceIconSegment);
    } else if (component_id == Component::kPoint) {
        const auto &parent_component_id = parentComponent();
        if (parent_component_id == Component::kRail) {
            return QIcon(ResourceIconRailPoint);
        } else if (parent_component_id == Component::kSegment) {
            return QIcon(ResourceIconRailPoint);
        } else {
            return QIcon(ResourceIconPoint);
        }
    } else {
        qWarning() << CLASSNAME << "[Warning] fail getting displayed icon, missing definition" << component_id;
        return QVariant();
    }
}

// Returns the component identifier of the underlying protobuf message.
ModelBacken::Component ModelBacken::component() const
{
    return component(_protobuf);
}

// Returns the component identifier of the underlying protobuf message for the
// parent item.
ModelBacken::Component ModelBacken::parentComponent() const
{
    if (!_item || !_item->parent()) {
        qWarning() << CLASSNAME << "[Warning] fail getting parent component identifier, null item pointer";
        return Component::kNoComponent;
    }

    return _item->parent()->backend().component();
}

// Returns the collection identifier of the underlying protobuf message.
// The collection identifier type is figured out by looking at the item
// children component identifier.
//  - A Route is a collection of Point.
//  - A Family is a collection of Rail.
ModelBacken::Collection ModelBacken::collection() const
{
    if (!_item) {
        qWarning() << CLASSNAME << "[Warning] fail getting collection identifier, null item pointer";
        return Collection::kScenario;
    }

    if (_item->childCount()) {
        auto is_route = true;
        auto is_family = true;
        for (auto *child_item : _item->_childs) {
            const auto &component_id = child_item->backend().component();
            is_route &= component_id == Component::kPoint;
            is_family &= component_id == Component::kRail;
        }
        if (is_route) {
            return Collection::kRoute;
        } else if (is_family) {
            return Collection::kFamily;
        }
    }
    return Collection::kScenario;
}

// Returns the authorized action of the underlying protobuf message. This
// depends on the component and parent component identifer of the underlying
// protobuf message.
unsigned int ModelBacken::authorization() const
{
    const auto &component_id = component();

    if (component_id == ModelBacken::kMission) {
        return (1 << kDelete) | (1 << kPoint) | (1 << kRail) | (1 << kSegment) | (1 << kCollection);

    } else if (component_id == ModelBacken::kCollection) {
        return (1 << kDelete) | (1 << kPoint) | (1 << kRail) | (1 << kSegment);

    } else if (component_id == ModelBacken::kRail || component_id == ModelBacken::kSegment) {
        return (1 << kDelete);

    } else if (component_id == ModelBacken::kPoint) {
        const auto &parent_component_id = parentComponent();
        if (parent_component_id != ModelBacken::kRail && parent_component_id != ModelBacken::kSegment) {
            return (1 << kDelete);
        }

    } else {
        qWarning() << CLASSNAME << "[Warning] fail getting authorized action, missing definition" << component_id;
    }

    return 0;
}

// Remove the row-protobuf message of the underlying protobuf message specified by the row.
bool ModelBacken::removeRow(const int row)
{
    const auto &component_id = component();

    if (component_id == ModelBacken::kMission) {
        RepeatedFieldRemoveAt(row, static_cast<pb::mission::Mission *>(_protobuf)->mutable_components());

    } else if (component_id == ModelBacken::kCollection) {
        RepeatedFieldRemoveAt(row, static_cast<pb::mission::Mission::Collection *>(_protobuf)->mutable_elements());

    } else if (component_id == ModelBacken::kNoComponent) {
        // In this case we want to remove a top-level row-protobuf message, it
        // means this backend is the one linked to the root item. In order to
        // remove the row-protobuf message, we first retrieve the child item
        // specified by the row and then clear the underlying protobuf message.
        if (_item->childCount() >= row) {
            _item->child(row)->backend().clear();
        } else {
            qWarning() << CLASSNAME << "[Warning] fail removing row, missing item child" << row;
            return false;
        }

    } else {
        qWarning() << CLASSNAME << "[Warning] fail removing row, missing definition" << component_id;
        return false;
    }

    return true;
}

// Clears the underlying protobuf message.
void ModelBacken::clear()
{
    if (!_protobuf) {
        qWarning() << CLASSNAME << "[Warning] fail clearing, null protobuf pointer";
        return;
    }

    _protobuf->Clear();
}

// Creates and appends a protobuf element into the underlying protobuf message.
// The created protobuf message depends on the specified action.
google::protobuf::Message *ModelBacken::appendRow(const Component new_component)
{
    if (!isAuthorized(new_component)) {
        qWarning() << CLASSNAME << "[Warning] fail appending row, action not authorized";
        return nullptr;
    }

    const auto &component_id = component();

    if (new_component == kCollection) {
        if (component_id == kMission) {
            auto *protobuf = static_cast<pb::mission::Mission *>(_protobuf)->add_components()->mutable_collection();
            protobuf->set_name(QString("Collection %1").arg(_item->childCount()).toStdString());
            return protobuf;
        } else {
            qWarning() << CLASSNAME << "[Warning] fail appending row, can't add a collection into component"
                       << component_id;
            return nullptr;
        }

    } else if (new_component == kPoint) {
        if (component_id == kMission) {
            auto *protobuf = static_cast<pb::mission::Mission *>(_protobuf)->add_components()->mutable_element();
            protobuf->mutable_point()->set_name(QString("Point %1").arg(_item->childCount()).toStdString());
            return protobuf;
        } else if (component_id == kCollection) {
            auto *protobuf = static_cast<pb::mission::Mission::Collection *>(_protobuf)->add_elements();
            protobuf->mutable_point()->set_name(QString("Point %1").arg(_item->childCount()).toStdString());
            return protobuf;
        } else {
            qWarning() << CLASSNAME << "[Warning] fail appending row, can't add a point into component" << component_id;
            return nullptr;
        }

    } else if (new_component == kRail) {
        if (component_id == kMission) {
            auto *protobuf = static_cast<pb::mission::Mission *>(_protobuf)->add_components()->mutable_element();
            protobuf->mutable_rail()->set_name(QString("Rail %1").arg(_item->childCount()).toStdString());
            protobuf->mutable_rail()->mutable_p0()->set_name("JA");
            protobuf->mutable_rail()->mutable_p1()->set_name("JB");
            return protobuf;
        } else if (component_id == kCollection) {
            auto *protobuf = static_cast<pb::mission::Mission::Collection *>(_protobuf)->add_elements();
            protobuf->mutable_rail()->set_name(QString("Rail %1").arg(_item->childCount()).toStdString());
            protobuf->mutable_rail()->mutable_p0()->set_name("JA");
            protobuf->mutable_rail()->mutable_p1()->set_name("JB");
            return protobuf;
        } else {
            qWarning() << CLASSNAME << "[Warning] fail appending row, can't add a rail into component" << component_id;
            return nullptr;
        }

    } else if (new_component == kSegment) {
        if (component_id == kMission) {
            auto *protobuf = static_cast<pb::mission::Mission *>(_protobuf)->add_components()->mutable_element();
            protobuf->mutable_segment()->set_name(QString("Segment %1").arg(_item->childCount()).toStdString());
            protobuf->mutable_segment()->mutable_p0()->set_name("SA");
            protobuf->mutable_segment()->mutable_p1()->set_name("SB");
            return protobuf;
        } else if (component_id == kCollection) {
            auto *protobuf = static_cast<pb::mission::Mission::Collection *>(_protobuf)->add_elements();
            protobuf->mutable_segment()->set_name(QString("Segment %1").arg(_item->childCount()).toStdString());
            protobuf->mutable_segment()->mutable_p0()->set_name("SA");
            protobuf->mutable_segment()->mutable_p1()->set_name("SB");
            return protobuf;
        } else {
            qWarning() << CLASSNAME << "[Warning] fail appending row, can't add a segment into component"
                       << component_id;
            return nullptr;
        }

    } else {
        qWarning() << CLASSNAME << "[Warning] fail appending row, missing definition" << new_component;
    }

    return nullptr;
}

// TODO
bool ModelBacken::moveLastAt(const int row)
{
    const auto &component_id = component();

    if (component_id == ModelBacken::kMission) {
        RepeatedFieldMoveLastAt(row, static_cast<pb::mission::Mission *>(_protobuf)->mutable_components());

    } else if (component_id == ModelBacken::kCollection) {
        RepeatedFieldMoveLastAt(row, static_cast<pb::mission::Mission::Collection *>(_protobuf)->mutable_elements());

    } else if (component_id == ModelBacken::kNoComponent) {
        // In this case we want to remove a top-level row-protobuf message, it
        // means this backend is the one linked to the root item. In order to
        // remove the row-protobuf message, we first retrieve the child item
        // specified by the row and then clear the underlying protobuf message.
        qCritical() << CLASSNAME << "[Critical] fail inserting row, not implemented"; // TODO
        return false;

    } else {
        qWarning() << CLASSNAME << "[Warning] fail removing row, missing definition" << component_id;
        return false;
    }

    return true;
}

// Returns the data specified by the column of the underlying protobuf message.
QVariant ModelBacken::data(const int column) const
{
    // Here we use some king to inline template function thank to C++14 (lambda function with auto parameter);
    auto getter = [&](auto *message) -> QVariant {
        if (column == 0) { // Composant Name
            return QString::fromStdString(message->GetDescriptor()->name());
        } else if (column == 1) { // User Name
            return QString::fromStdString(message->name());
        }
        return QVariant("No Column");
    };

    const auto &component_id = component();

    if (component_id == ModelBacken::kMission) {
        return getter(static_cast<pb::mission::Mission *>(_protobuf));

    } else if (component_id == ModelBacken::kCollection) {
        return getter(static_cast<pb::mission::Mission::Collection *>(_protobuf));

    } else if (component_id == ModelBacken::kPoint) {
        return getter(static_cast<pb::mission::Mission::Element::Point *>(_protobuf));

    } else if (component_id == ModelBacken::kRail) {
        return getter(static_cast<pb::mission::Mission::Element::Rail *>(_protobuf));

    } else if (component_id == ModelBacken::kSegment) {
        return getter(static_cast<pb::mission::Mission::Element::Segment *>(_protobuf));

    } else {
        qWarning() << CLASSNAME << "[Warning] fail getting data, missing definition" << component_id;
        return QVariant("No Component ID");
    }
}

// This sets the data specified by the column of the underlying protobuf message.
bool ModelBacken::setData(int column, const QVariant &value)
{
    // Here we use some king to inline template function thank to C++14 (lambda function with auto parameter);
    auto setter = [&](auto *message) {
        if (column == 1) { // User Name
            message->set_name(value.toString().toStdString());
            return true;
        }
        return false;
    };

    const auto &component_id = component();

    if (component_id == ModelBacken::kMission) {
        return setter(static_cast<pb::mission::Mission *>(_protobuf));

    } else if (component_id == ModelBacken::kCollection) {
        return setter(static_cast<pb::mission::Mission::Collection *>(_protobuf));

    } else if (component_id == ModelBacken::kPoint) {
        return setter(static_cast<pb::mission::Mission::Element::Point *>(_protobuf));

    } else if (component_id == ModelBacken::kRail) {
        return setter(static_cast<pb::mission::Mission::Element::Rail *>(_protobuf));

    } else if (component_id == ModelBacken::kSegment) {
        return setter(static_cast<pb::mission::Mission::Element::Segment *>(_protobuf));

    } else {
        qWarning() << CLASSNAME << "[Warning] fail setting data, missing definition" << component_id;
        return false;
    }
}
