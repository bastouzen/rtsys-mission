// ===
// === Include
// ============================================================================ //

#include "mission/item.h"
#include "protobuf/mission.pb.h"

#include <QDebug>
#include <QIcon>

// ===
// === Define
// ============================================================================ //

#define CLASSNAME "ModelItem ::"

const auto ResourceIconMission = QStringLiteral(":/resource/mission.svg");
const auto ResourceIconCollection = QStringLiteral(":/resource/collection.svg");
const auto ResourceIconRoute = QStringLiteral(":/resource/route.svg");
const auto ResourceIconFamily = QStringLiteral(":/resource/family.svg");
const auto ResourceIconRail = QStringLiteral(":/resource/rail.svg");
const auto ResourceIconSegment = QStringLiteral(":/resource/segment.svg");
const auto ResourceIconPoint = QStringLiteral(":/resource/point.png");
const auto ResourceIconRailPoint = QStringLiteral(":/resource/rail.png");

const auto FlagMission = pb::mission::Mission::descriptor() -> name();
const auto FlagComponant = pb::mission::Mission::Component::descriptor() -> name();
const auto FlagCollection = pb::mission::Mission::Collection::descriptor() -> name();
const auto FlagElement = pb::mission::Mission::Element::descriptor() -> name();
const auto FlagPoint = pb::mission::Mission::Element::Point::descriptor() -> name();
const auto FlagRail = pb::mission::Mission::Element::Rail::descriptor() -> name();
const auto FlagSegment = pb::mission::Mission::Element::Segment::descriptor() -> name();

// ===
// === Function
// ============================================================================ //

// Returns the flag identifier specified by the underlying protobuf message.
ModelItem::Flag ModelItem::flag(const Protobuf *protobuf)
{
    if (!protobuf) return Flag::kUndefined;

    const auto &name = protobuf->GetDescriptor()->name();

    if (name == FlagMission) {
        return Flag::kMission;
    } else if (name == FlagComponant) {
        return Flag::kComponent;
    } else if (name == FlagCollection) {
        return Flag::kCollection;
    } else if (name == FlagElement) {
        return Flag::kElement;
    } else if (name == FlagPoint) {
        return Flag::kPoint;
    } else if (name == FlagRail) {
        return Flag::kRail;
    } else if (name == FlagSegment) {
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

// Creates an item into the parent children and then set its data from the specified
// protobuf message.
inline ModelItem *addItem(ModelItem::Protobuf *protobuf, ModelItem *parent)
{
    parent->insertChild();                                // add child at last position
    auto *item = parent->child(parent->countChild() - 1); // retrieve the created item and set it
    item->setData(QVariant::fromValue(ModelItem::wrap(protobuf)), Qt::UserRoleWrapper);
    return item;
}

// Creates an point item into the parent children with the specified protobuf message.
inline ModelItem *addPoint(pb::mission::Mission::Element::Point *point, ModelItem *parent)
{
    return addItem(point, parent);
}

// Creates an rail item into the parent children with the specified protobuf message.
inline ModelItem *addRail(pb::mission::Mission::Element::Rail *rail, ModelItem *parent)
{
    auto *item = addItem(rail, parent);
    addItem(rail->mutable_p0(), item);
    addItem(rail->mutable_p1(), item);
    return item;
}

// Creates an segment item into the parent children with the specified protobuf message.
inline ModelItem *addSegment(pb::mission::Mission::Element::Segment *segment, ModelItem *parent)
{
    auto *item = addItem(segment, parent);
    addItem(segment->mutable_p0(), item);
    addItem(segment->mutable_p1(), item);
    return item;
}

// Creates an element item into the parent children with the specified protobuf message.
inline ModelItem *addElement(pb::mission::Mission::Element *element, ModelItem *parent)
{
    ModelItem *item = nullptr;
    switch (element->element_case()) {
        case pb::mission::Mission::Element::kPoint:
            item = addPoint(element->mutable_point(), parent);
            break;
        case pb::mission::Mission::Element::kRail:
            item = addRail(element->mutable_rail(), parent);
            break;
        case pb::mission::Mission::Element::kSegment:
            item = addSegment(element->mutable_segment(), parent);
            break;
        default:
            qWarning() << CLASSNAME << "[Warning] fail appending row element, missing definition"
                       << element->element_case();
            break;
    }
    return item;
};

// Creates an collection item into the parent children with the specified protobuf message.
inline ModelItem *addCollection(pb::mission::Mission::Collection *collection, ModelItem *parent)
{
    auto item = addItem(collection, parent);
    for (auto &element : *collection->mutable_elements()) {
        addElement(&element, item);
    }
    return item;
};

// Creates an mission item into the parent children with the specified protobuf message.
inline ModelItem *addMission(pb::mission::Mission *mission, ModelItem *parent)
{
    auto item = addItem(mission, parent);
    for (auto &component : *mission->mutable_components()) {
        switch (component.component_case()) {
            case pb::mission::Mission::Component::kElement:
                addElement(component.mutable_element(), item);
                break;
            case pb::mission::Mission::Component::kCollection:
                addCollection(component.mutable_collection(), item);
                break;
            default:
                break;
        }
    }
    return item;
};

// ===
// === Class
// ============================================================================ //

ModelItem::ModelItem(ModelItem *parent)
    : _parent(parent)
    , _protobuf(nullptr)
{
}

ModelItem::~ModelItem()
{
    qDeleteAll(_childs);
}

// Returns the supported flag of item. This depends on the item flag and parent
// item flag.
unsigned int ModelItem::supportedFlags() const
{
    const auto &flag_id = flag(_protobuf);

    if (flag_id == kMission) {
        return (1 << kDelete) | (1 << kPoint) | (1 << kRail) | (1 << kSegment) | (1 << kCollection);

    } else if (flag_id == kCollection) {
        return (1 << kDelete) | (1 << kPoint) | (1 << kRail) | (1 << kSegment);

    } else if (flag_id == kRail || flag_id == kSegment) {
        return (1 << kDelete);

    } else if (flag_id == kPoint) {
        const auto &parent_flag_id = flag(_parent->protobuf());
        if (parent_flag_id != kRail && parent_flag_id != kSegment) {
            return (1 << kDelete);
        }

    } else {
        qWarning() << CLASSNAME << "[Warning] fail getting suported flags, missing definition" << flag_id;
    }

    return 0;
}

// Removes the child item specified by the given row.
void ModelItem::removeChild(int row)
{
    Q_ASSERT(!(row < 0 || row >= _childs.size()));

    auto removeProtobuf = [&]() {
        // Remove the child protobuf message.
        const auto &flag_id = flag(_protobuf);
        if (flag_id == kMission) {
            RepeatedFieldRemoveAt(row, static_cast<pb::mission::Mission *>(_protobuf)->mutable_components());

        } else if (flag_id == kCollection) {
            RepeatedFieldRemoveAt(row, static_cast<pb::mission::Mission::Collection *>(_protobuf)->mutable_elements());

        } else if (flag_id == kUndefined) {
            // In this case we want to remove a top-level row-protobuf message, it
            // means this backend is the one linked to the root item. In order to
            // remove the row-protobuf message, we first retrieve the child item
            // specified by the row and then clear the underlying protobuf message.
            if (countChild() >= row) {
                child(row)->protobuf()->Clear();
            } else {
                qWarning() << CLASSNAME << "[Warning] fail removing row, missing item child" << row;
                return false;
            }

        } else {
            qWarning() << CLASSNAME << "[Warning] fail removing row, missing definition" << flag_id;
            return false;
        }
        return true;
    };

    removeProtobuf();
    auto *pointer = child(row);
    _childs.remove(row);
    delete pointer;
    pointer = nullptr;
}

// Inserts the child item specified by the given row.
void ModelItem::insertChild(int row)
{
    if (row >= 0 && row < countChild()) {
        _childs.insert(row, new ModelItem(this));
    } else {
        _childs.append(new ModelItem(this));
    }
}

// Sets the data for the specified value and role. Returns true if successful
// otherwise returns false.
bool ModelItem::setData(const QVariant &value, int role)
{
    // First of all we consume the wrapper if available.
    if (role == Qt::UserRoleWrapper) {
        ModelItem::Wrapper w = value.value<ModelItem::Wrapper>();
        qDebug() << w.pointer;
        setProtobuf(w.pointer);
        return true;

    } else if (role == Qt::UserRoleFlag) {
        setProtobuf(static_cast<Flag>(value.toInt()));
        return true;

    } else {
        const auto &flag_id = flag(_protobuf);
        if (flag_id == kMission) {
            static_cast<pb::mission::Mission *>(_protobuf)->set_name(value.toString().toStdString());
            return true;
        } else if (flag_id == kCollection) {
            static_cast<pb::mission::Mission::Collection *>(_protobuf)->set_name(value.toString().toStdString());
            return true;
        } else if (flag_id == kPoint) {
            static_cast<pb::mission::Mission::Element::Point *>(_protobuf)->set_name(value.toString().toStdString());
            return true;
        } else if (flag_id == kRail) {
            static_cast<pb::mission::Mission::Element::Rail *>(_protobuf)->set_name(value.toString().toStdString());
            return true;
        } else if (flag_id == kSegment) {
            static_cast<pb::mission::Mission::Element::Segment *>(_protobuf)->set_name(value.toString().toStdString());
            return true;
        } else {
            qWarning() << CLASSNAME << "[Warning] fail setting data, missing flag" << flag_id;
            return false;
        }
    }
}

// Returns the data specified by the role.
QVariant ModelItem::data(const int role) const
{
    // Here we use some king to inline template function thank to C++14
    // "lambda function with auto parameter"
    auto getter = [&](auto *message) -> QVariant {
        if (!message) return QVariant("Null Pointer");
        if (role == Qt::UserRoleFlagId) {
            return QString::fromStdString(message->GetDescriptor()->name());
        } else if (role == Qt::UserRoleFlagName) {
            return QString::fromStdString(message->name());
        } else {
            return QVariant("Miss. Role");
        }
    };

    // First of all we consume the wrapper if available.
    if (role == Qt::DecorationRole) {
        return icon();

    } else {
        const auto &flag_id = flag(_protobuf);
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
            qWarning() << CLASSNAME << "[Warning] fail getting data, missing flag" << flag_id;
            return QVariant("Miss. Flag");
        }
    }
}

// Returns the displayed icon of the item. The icon is figured out by looking at
// the flag and flag collection identifier.
QVariant ModelItem::icon() const
{
    const auto &flag_id = flag(_protobuf);
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
        const auto &pareng_flag_id = flag(_parent->protobuf());
        if (pareng_flag_id == kRail) {
            return QIcon(ResourceIconRailPoint);
        } else if (pareng_flag_id == kSegment) {
            return QIcon(ResourceIconRailPoint);
        } else {
            return QIcon(ResourceIconPoint);
        }
    } else {
        qWarning() << CLASSNAME << "[Warning] fail getting displayed icon, missing flag" << flag_id;
        return QVariant();
    }
}

// Returns the flag collection identifier of the item. The flag collection
// identifier is figured out by looking at the item children flag identifier
//  - A Route is a collection of Point.
//  - A Family is a collection of Rail.
ModelItem::Flag ModelItem::collectionFlag() const
{
    if (countChild()) {
        auto is_route = true;
        auto is_family = true;
        for (auto *child_item : _childs) {
            const auto &flag_id = flag(child_item->protobuf());
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

// Sets the part-protobuf message specified flag identifier.
void ModelItem::setProtobuf(const ModelItem::Flag new_flag)
{
    if (!_parent->isFlagSupported(new_flag)) {
        qWarning() << CLASSNAME << "[Warning] fail setting protobuf, flag not supported";
        return;
    }

    const auto &parent_flag_id = ModelItem::flag(_parent->protobuf());
    const auto &parent_count_child = _parent->countChild() - 1;

    auto add_point = [&]() {
        Protobuf *protobuf = nullptr;
        if (parent_flag_id == ModelItem::kMission) {
            protobuf = static_cast<pb::mission::Mission *>(_parent->_protobuf)
                           ->add_components()
                           ->mutable_element()
                           ->mutable_point();
        }
        if (parent_flag_id == ModelItem::kCollection) {
            protobuf =
                static_cast<pb::mission::Mission::Collection *>(_parent->_protobuf)->add_elements()->mutable_point();
        }
        return protobuf;
    };

    auto add_rail = [&]() {
        Protobuf *protobuf = nullptr;
        if (parent_flag_id == ModelItem::kMission) {
            protobuf = static_cast<pb::mission::Mission *>(_parent->_protobuf)
                           ->add_components()
                           ->mutable_element()
                           ->mutable_rail();
        }
        if (parent_flag_id == ModelItem::kCollection) {
            protobuf =
                static_cast<pb::mission::Mission::Collection *>(_parent->_protobuf)->add_elements()->mutable_rail();
        }
        return protobuf;
    };

    auto add_segment = [&]() {
        Protobuf *protobuf = nullptr;
        if (parent_flag_id == ModelItem::kMission) {
            protobuf = static_cast<pb::mission::Mission *>(_parent->_protobuf)
                           ->add_components()
                           ->mutable_element()
                           ->mutable_segment();
        }
        if (parent_flag_id == ModelItem::kCollection) {
            protobuf =
                static_cast<pb::mission::Mission::Collection *>(_parent->_protobuf)->add_elements()->mutable_segment();
        }
        return protobuf;
    };

    if (new_flag == ModelItem::kCollection) {
        if (parent_flag_id == kMission) {
            auto *protobuf =
                static_cast<pb::mission::Mission *>(_parent->_protobuf)->add_components()->mutable_collection();
            protobuf->set_name(QString("Collection %1").arg(parent_count_child).toStdString());
            _protobuf = protobuf;
        } else {
            qWarning() << CLASSNAME << "[Warning] fail setting protobuf, can't add a collection into flag"
                       << parent_flag_id;
        }

    } else if (new_flag == ModelItem::kPoint) {
        auto *protobuf = static_cast<pb::mission::Mission::Element::Point *>(add_point());
        if (protobuf) {
            protobuf->set_name(QString("Point %1").arg(parent_count_child).toStdString());
            _protobuf = protobuf;
        } else {
            qWarning() << CLASSNAME << "[Warning] fail setting protobuf, can't add a point into flag" << parent_flag_id;
        }

    } else if (new_flag == ModelItem::kRail) {
        auto *protobuf = static_cast<pb::mission::Mission::Element::Rail *>(add_rail());
        if (protobuf) {
            protobuf->set_name(QString("Rail %1").arg(parent_count_child).toStdString());
            protobuf->mutable_p0()->set_name("JA");
            protobuf->mutable_p1()->set_name("JB");
            _protobuf = protobuf;
            addItem(protobuf->mutable_p0(), this);
            addItem(protobuf->mutable_p1(), this);
        } else {
            qWarning() << CLASSNAME << "[Warning] fail setting protobuf, can't add a rail into flag" << parent_flag_id;
        }

    } else if (new_flag == ModelItem::kSegment) {
        auto *protobuf = static_cast<pb::mission::Mission::Element::Segment *>(add_segment());
        if (protobuf) {
            protobuf->set_name(QString("Segment %1").arg(parent_count_child).toStdString());
            protobuf->mutable_p0()->set_name("SA");
            protobuf->mutable_p1()->set_name("SB");
            _protobuf = protobuf;
            addItem(protobuf->mutable_p0(), this);
            addItem(protobuf->mutable_p1(), this);
        } else {
            qWarning() << CLASSNAME << "[Warning] fail setting protobuf, can't add a rail into flag" << parent_flag_id;
        }

    } else {
        qWarning() << CLASSNAME << "[Warning] fail setting protobuf, missing flag" << new_flag;
    }
}

// Appends a child into the parent children with the specified underlying protobuf
// message.

void ModelItem::setProtobuf(Protobuf *protobuf)
{
    if (!protobuf) {
        qWarning() << CLASSNAME << "[Warning] fail appending child, null protobuf pointer";
        return;
    }

    const auto &flag_id = flag(protobuf);

    if (flag_id == kMission) {
        addMission(static_cast<pb::mission::Mission *>(protobuf), this);

    } else if (flag_id == kCollection) {
        addCollection(static_cast<pb::mission::Mission::Collection *>(protobuf), this);

    } else if (flag_id == kElement) {
        addElement(static_cast<pb::mission::Mission::Element *>(protobuf), this);

    } else if (flag_id == kPoint) {
        addPoint(static_cast<pb::mission::Mission::Element::Point *>(protobuf), this);

    } else if (flag_id == kRail) {
        addRail(static_cast<pb::mission::Mission::Element::Rail *>(protobuf), this);

    } else if (flag_id == kSegment) {
        addSegment(static_cast<pb::mission::Mission::Element::Segment *>(protobuf), this);

    } else {
        qWarning() << CLASSNAME << "[Warning] fail setting protobuf, missing flag" << flag_id;
    }
}
