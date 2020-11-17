// ===
// === Include
// ============================================================================ //

#include "mission/backend.h"
#include "mission/item.h"
#include "protobuf/misc/misc_cpp.h"
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
const auto ComponentTypeComponant = pb::mission::Mission::Component::descriptor() -> name();
const auto ComponentTypeCollection = pb::mission::Mission::Collection::descriptor() -> name();
const auto ComponentTypeElement = pb::mission::Mission::Element::descriptor() -> name();
const auto ComponentTypePoint = pb::mission::Mission::Element::Point::descriptor() -> name();
const auto ComponentTypeRail = pb::mission::Mission::Element::Rail::descriptor() -> name();
const auto ComponentTypeSegment = pb::mission::Mission::Element::Segment::descriptor() -> name();

// ===
// === Function
// ============================================================================ //

// Returns the flag identifier specified by the underlying protobuf message.
ModelBacken::Flag ModelBacken::flag(const Protobuf *protobuf)
{
    if (!protobuf) return Flag::kUndefined;

    const auto &name = protobuf->GetDescriptor()->name();

    if (name == ComponentTypeMission) {
        return Flag::kMission;
    } else if (name == ComponentTypeComponant) {
        return Flag::kComponent;
    } else if (name == ComponentTypeCollection) {
        return Flag::kCollection;
    } else if (name == ComponentTypeElement) {
        return Flag::kElement;
    } else if (name == ComponentTypePoint) {
        return Flag::kPoint;
    } else if (name == ComponentTypeRail) {
        return Flag::kRail;
    } else if (name == ComponentTypeSegment) {
        return Flag::kSegment;
    } else {
        qWarning() << CLASSNAME << "[Warning] fail getting flag identifier, missing definition" << name.data();
        return Flag::kUndefined;
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
// repeated field doesn't provide any MoveAt function we use the SwapElements
// function.
template <class T>
void RepeatedFieldMoveUpLastAt(const int row, google::protobuf::RepeatedPtrField<T> *repeated)
{
    for (int i = repeated->size(); i < row; i--) {
        repeated->SwapElements(i, i - 1);
    }
}

// ===
// === Class
// ============================================================================ //

ModelBacken::ModelBacken(ModelItem *item)
    : _protobuf(nullptr)
    , _item(item)
{
}

// Returns the displayed icon of the underlying protobuf message. The icon
// is figured out by looking at the componenet and collection type.
QVariant ModelBacken::icon() const
{
    const auto &flag_id = flag();
    if (flag_id == kMission) {
        return QIcon(ResourceIconMission);
    } else if (flag_id == kCollection) {
        const auto &collection_flag_id = collectionFlag();
        if (collection_flag_id == kRoute) {
            return QIcon(ResourceIconRoute);
        } else if (collection_flag_id == kFamily) {
            return QIcon(ResourceIconFamily);
        } else {
            return QIcon(ResourceIconCollection);
        }
    } else if (flag_id == kRail) {
        return QIcon(ResourceIconRail);
    } else if (flag_id == kSegment) {
        return QIcon(ResourceIconSegment);
    } else if (flag_id == kPoint) {
        const auto &pareng_flag_id = parentFlag();
        if (pareng_flag_id == kRail) {
            return QIcon(ResourceIconRailPoint);
        } else if (pareng_flag_id == kSegment) {
            return QIcon(ResourceIconRailPoint);
        } else {
            return QIcon(ResourceIconPoint);
        }
    } else {
        qWarning() << CLASSNAME << "[Warning] fail getting displayed icon, missing definition" << flag_id;
        return QVariant();
    }
}

// Returns the component identifier of the underlying protobuf message.
ModelBacken::Flag ModelBacken::flag() const
{
    return flag(_protobuf);
}

// Returns the component identifier of the underlying protobuf message for the
// parent item.
ModelBacken::Flag ModelBacken::parentFlag() const
{
    if (!_item || !_item->parent()) {
        qWarning() << CLASSNAME << "[Warning] fail getting parent flag identifier, null item pointer";
        return Flag::kUndefined;
    }

    return _item->parent()->flag();
}

// Returns the flag collection identifier of the underlying protobuf message.
// The flag collection identifier type is figured out by looking at the item
// children flag identifier.
//  - A Route is a collection of Point.
//  - A Family is a collection of Rail.
ModelBacken::Flag ModelBacken::collectionFlag() const
{
    if (!_item) {
        qWarning() << CLASSNAME << "[Warning] fail getting flag collection identifier, null item pointer";
        return kScenario;
    }

    if (_item->countChild()) {
        auto is_route = true;
        auto is_family = true;
        for (auto *child_item : _item->_childs) {
            const auto &flag_id = child_item->backend().flag();
            is_route &= flag_id == kPoint;
            is_family &= flag_id == kRail;
        }
        if (is_route) {
            return kRoute;
        } else if (is_family) {
            return kFamily;
        }
    }
    return kScenario;
}

// Returns the supported flag of the underlying protobuf message. This depends
// on the component and parent component identifer of the underlying protobuf
// message.
unsigned int ModelBacken::supportedFlags() const
{
    const auto &flag_id = flag();

    if (flag_id == kMission) {
        return (1 << kDelete) | (1 << kPoint) | (1 << kRail) | (1 << kSegment) | (1 << kCollection);

    } else if (flag_id == kCollection) {
        return (1 << kDelete) | (1 << kPoint) | (1 << kRail) | (1 << kSegment);

    } else if (flag_id == kRail || flag_id == kSegment) {
        return (1 << kDelete);

    } else if (flag_id == kPoint) {
        const auto &parent_flag_id = parentFlag();
        if (parent_flag_id != kRail && parent_flag_id != kSegment) {
            return (1 << kDelete);
        }

    } else {
        qWarning() << CLASSNAME << "[Warning] fail getting suported flags, missing definition" << flag_id;
    }

    return 0;
}

// Remove the part-protobuf message of the underlying protobuf message
// specified by the row.
bool ModelBacken::removeRow(const int row)
{
    const auto &flag_id = flag();

    if (flag_id == kMission) {
        RepeatedFieldRemoveAt(row, static_cast<pb::mission::Mission *>(_protobuf)->mutable_components());

    } else if (flag_id == kCollection) {
        RepeatedFieldRemoveAt(row, static_cast<pb::mission::Mission::Collection *>(_protobuf)->mutable_elements());

    } else if (flag_id == kUndefined) {
        // In this case we want to remove a top-level row-protobuf message, it
        // means this backend is the one linked to the root item. In order to
        // remove the row-protobuf message, we first retrieve the child item
        // specified by the row and then clear the underlying protobuf message.
        if (_item->countChild() >= row) {
            _item->child(row)->backend().protobuf()->Clear();
        } else {
            qWarning() << CLASSNAME << "[Warning] fail removing row, missing item child" << row;
            return false;
        }

    } else {
        qWarning() << CLASSNAME << "[Warning] fail removing row, missing definition" << flag_id;
        return false;
    }

    return true;
}

// Appends a part-protobuf message into the underlying protobuf message.
// This creates the part-protobuf message depending on the specified new_flag.
ModelBacken::Protobuf *ModelBacken::appendRow(const Flag new_flag)
{
    if (!canSupportFlag(new_flag)) {
        qWarning() << CLASSNAME << "[Warning] fail appending row, flag not supported";
        return nullptr;
    }

    const auto &flag_id = flag();

    if (new_flag == kCollection) {
        if (flag_id == kMission) {
            auto *protobuf = static_cast<pb::mission::Mission *>(_protobuf)->add_components()->mutable_collection();
            protobuf->set_name(QString("Collection %1").arg(_item->countChild()).toStdString());
            return protobuf;
        } else {
            qWarning() << CLASSNAME << "[Warning] fail appending row, can't add a collection into flag" << flag_id;
            return nullptr;
        }

    } else if (new_flag == kPoint) {
        if (flag_id == kMission) {
            auto *protobuf =
                static_cast<pb::mission::Mission *>(_protobuf)->add_components()->mutable_element()->mutable_point();
            protobuf->set_name(QString("Point %1").arg(_item->countChild()).toStdString());
            return protobuf;
        } else if (flag_id == kCollection) {
            auto *protobuf =
                static_cast<pb::mission::Mission::Collection *>(_protobuf)->add_elements()->mutable_point();
            protobuf->set_name(QString("Point %1").arg(_item->countChild()).toStdString());
            return protobuf;
        } else {
            qWarning() << CLASSNAME << "[Warning] fail appending row, can't add a point into flag" << flag_id;
            return nullptr;
        }

    } else if (new_flag == kRail) {
        if (flag_id == kMission) {
            auto *protobuf =
                static_cast<pb::mission::Mission *>(_protobuf)->add_components()->mutable_element()->mutable_rail();
            protobuf->set_name(QString("Rail %1").arg(_item->countChild()).toStdString());
            protobuf->mutable_p0()->set_name("JA");
            protobuf->mutable_p1()->set_name("JB");
            return protobuf;
        } else if (flag_id == kCollection) {
            auto *protobuf = static_cast<pb::mission::Mission::Collection *>(_protobuf)->add_elements()->mutable_rail();
            protobuf->set_name(QString("Rail %1").arg(_item->countChild()).toStdString());
            protobuf->mutable_p0()->set_name("JA");
            protobuf->mutable_p1()->set_name("JB");
            return protobuf;
        } else {
            qWarning() << CLASSNAME << "[Warning] fail appending row, can't add a rail into flag" << flag_id;
            return nullptr;
        }

    } else if (new_flag == kSegment) {
        if (flag_id == kMission) {
            auto *protobuf =
                static_cast<pb::mission::Mission *>(_protobuf)->add_components()->mutable_element()->mutable_segment();
            protobuf->set_name(QString("Segment %1").arg(_item->countChild()).toStdString());
            protobuf->mutable_p0()->set_name("SA");
            protobuf->mutable_p1()->set_name("SB");
            return protobuf;
        } else if (flag_id == kCollection) {
            auto *protobuf =
                static_cast<pb::mission::Mission::Collection *>(_protobuf)->add_elements()->mutable_segment();
            protobuf->set_name(QString("Segment %1").arg(_item->countChild()).toStdString());
            protobuf->mutable_p0()->set_name("SA");
            protobuf->mutable_p1()->set_name("SB");
            return protobuf;
        } else {
            qWarning() << CLASSNAME << "[Warning] fail appending row, can't add a segment into flag" << flag_id;
            return nullptr;
        }

    } else {
        qWarning() << CLASSNAME << "[Warning] fail appending row, missing definition" << new_flag;
    }

    return nullptr;
}

// Inserts a part-protobuf message into the underlying protobuf message.
// This creates the part-protobuf message depending on the specified row and new_flag.
ModelBacken::Protobuf *ModelBacken::insertRow(const int row, const Flag new_flag)
{
    auto *protobuf = appendRow(new_flag);

    // This code above moves up the last row (i.e the appended row) to the specified
    // row index.
    const auto &flag_id = flag();
    if (flag_id == kMission) {
        RepeatedFieldMoveUpLastAt(row, static_cast<pb::mission::Mission *>(_protobuf)->mutable_components());
    } else if (flag_id == kCollection) {
        RepeatedFieldMoveUpLastAt(row, static_cast<pb::mission::Mission::Collection *>(_protobuf)->mutable_elements());
    } else if (flag_id == kUndefined) {
        // In this case we want to remove a top-level row-protobuf message, it
        // means this backend is the one linked to the root item.
        qCritical() << CLASSNAME << "[Critical] fail inserting row, not implemented"; // TODO
    } else {
        qWarning() << CLASSNAME << "[Warning] fail removing row, missing definition" << flag_id;
    }

    return protobuf;
}

// Returns the data specified by the role of the underlying protobuf message.
QVariant ModelBacken::data(const int role) const
{
    // Here we use some king to inline template function thank to C++14
    // "lambda function with auto parameter"
    auto getter = [&](auto *message) -> QVariant {
        if (!message) return QVariant("No Pointer");
        if (role == Qt::UserRoleFlagId) {
            return QString::fromStdString(message->GetDescriptor()->name());
        } else if (role == Qt::UserRoleFlagName) {
            return QString::fromStdString(message->name());
        } else if (role == Qt::UserRoleProtobufStream) {
            return QByteArray::fromStdString(rtsys::protobuf::misc::serializeDelimitedToString(*_protobuf));
        } else {
            return QVariant("No Role");
        }
    };

    const auto &flag_id = flag();

    if (flag_id == kMission) {
        return getter(static_cast<pb::mission::Mission *>(_protobuf));

    } else if (flag_id == kCollection) {
        return getter(static_cast<pb::mission::Mission::Collection *>(_protobuf));

    } else if (flag_id == kPoint) {
        return getter(static_cast<pb::mission::Mission::Element::Point *>(_protobuf));

    } else if (flag_id == kRail) {
        return getter(static_cast<pb::mission::Mission::Element::Rail *>(_protobuf));

    } else if (flag_id == kSegment) {
        return getter(static_cast<pb::mission::Mission::Element::Segment *>(_protobuf));

    } else {
        qWarning() << CLASSNAME << "[Warning] fail getting data, missing definition" << flag_id;
        return QVariant("No Flag");
    }
}

// Sets the data specified by the role of the underlying protobuf message.
bool ModelBacken::setData(const QVariant &value, int role)
{
    // Here we use some king to inline template function thank to C++14
    // "lambda function with auto parameter"
    auto setter = [&](auto *message) {
        if (role == Qt::UserRoleFlagName) {
            message->set_name(value.toString().toStdString());
            return true;
        } else if (role == Qt::UserRoleProtobufStream) {
            qWarning() << CLASSNAME << "[Warning] fail setting data, not implemented" << role; // TODO
            return false;
        } else {
            return false;
        }
    };

    const auto &flag_id = flag();

    if (flag_id == kMission) {
        return setter(static_cast<pb::mission::Mission *>(_protobuf));

    } else if (flag_id == kCollection) {
        return setter(static_cast<pb::mission::Mission::Collection *>(_protobuf));

    } else if (flag_id == kPoint) {
        return setter(static_cast<pb::mission::Mission::Element::Point *>(_protobuf));

    } else if (flag_id == kRail) {
        return setter(static_cast<pb::mission::Mission::Element::Rail *>(_protobuf));

    } else if (flag_id == kSegment) {
        return setter(static_cast<pb::mission::Mission::Element::Segment *>(_protobuf));

    } else {
        qWarning() << CLASSNAME << "[Warning] fail setting data, missing definition" << flag_id;
        return false;
    }
}
