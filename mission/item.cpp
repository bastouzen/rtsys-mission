// ===
// === Include
// ============================================================================ //

#include "mission/item_misc.h"
#include "protobuf/misc/misc_cpp.h"

#include <QIcon>
#include <QLoggingCategory>
#include <QMetaEnum>

// ===
// === Define
// ============================================================================ //

Q_LOGGING_CATEGORY(LC_RMI, "rtsys.mission.item")

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
MissionItem::Flag MissionItem::flag(const Protobuf *protobuf)
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
        qCWarning(LC_RMI) << "fail getting flag identifier, missing message [" << name.data() << "]";
        return Flag::kUndefined;
    }
}

QByteArray MissionItem::pack(const Protobuf &protobuf)
{
    return QByteArray::fromStdString(protobuf.SerializeAsString());
}

QSharedPointer<MissionItem::Protobuf> MissionItem::unpack(const QByteArray &array)
{
    QSharedPointer<MissionItem::Protobuf> pprotobuf;
    pprotobuf->ParseFromString(array.toStdString());
    // const auto flag_id = MissionItem::flag(pprotobuf.data());
    return pprotobuf;
}

// ===
// === Class
// ============================================================================ //

MissionItem::MissionItem(MissionItem *parent)
    : _parent(parent)
    , _protobuf(nullptr)
{
}

MissionItem::~MissionItem()
{
    qDeleteAll(_childs);
}

// Returns the supported flag of item. This depends on the item flag and parent
// item flag.
unsigned int MissionItem::supportedFlags() const
{
    const auto &flag_id = flag(_protobuf);

    if (flag_id == kUndefined) {
        return (1 << kMission);

    } else if (flag_id == kMission) {
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
        qCWarning(LC_RMI) << "fail getting supported flag, missing flag [" << flag_id << "]";
    }

    return 0;
}

// Removes the child item specified by the given row.
void MissionItem::removeChild(int row)
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
                qCWarning(LC_RMI) << "fail removing child, missing row [" << row << "]";
                return false;
            }

        } else {
            qCWarning(LC_RMI) << "fail removing child, missing flag [" << flag_id << "]";
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
void MissionItem::insertChild(int row)
{
    if (row >= 0 && row < countChild()) {
        _childs.insert(row, new MissionItem(this));
    } else {
        _childs.append(new MissionItem(this));
    }
}

// Sets the data for the specified value and role. Returns true if successful
// otherwise returns false.
bool MissionItem::setData(const QVariant &value, int role)
{
    if (role == Qt::UserRoleFlag) {
        setDataFromFlag(value.value<MissionItem::Flag>());
        return true;

    } else if (role == Qt::UserRoleName || role == Qt::EditRole) {
        // Here we use some king to inline template function thank to C++14
        // "lambda function with auto parameter"
        auto setUserRoleName = [&](auto *protobuf) { protobuf->set_name(value.toString().toStdString()); };

        const auto &flag_id = flag(_protobuf);
        if (flag_id == kMission) {
            setUserRoleName(static_cast<pb::mission::Mission *>(_protobuf));
        } else if (flag_id == kCollection) {
            setUserRoleName(static_cast<pb::mission::Mission::Collection *>(_protobuf));
        } else if (flag_id == kPoint) {
            setUserRoleName(static_cast<pb::mission::Mission::Element::Point *>(_protobuf));
        } else if (flag_id == kRail) {
            setUserRoleName(static_cast<pb::mission::Mission::Element::Rail *>(_protobuf));
        } else if (flag_id == kSegment) {
            setUserRoleName(static_cast<pb::mission::Mission::Element::Segment *>(_protobuf));
        } else {
            qCWarning(LC_RMI) << "fail setting data, missing flag [" << flag_id << "]";
            return false;
        }
        return true;

    } else if (role == Qt::UserRolePack) {
        setDataFromProtobuf(value.toByteArray());
        return true;
    }

    return false;
}

// Returns the data specified by the role.
QVariant MissionItem::data(const int role) const
{
    if (role == Qt::DecorationRole) {
        return icon();

    } else if (role == Qt::UserRoleFlag) {
        // Use reflection property of Qt for getting the name of the flag identifier.
        return QString(QMetaEnum::fromType<Flag>().valueToKey(flag(_protobuf))).mid(1, -1);

    } else if (role == Qt::UserRoleName) {
        // Here we use some king to inline template function thank to C++14
        // "lambda function with auto parameter"
        auto getUserName = [&](auto *message) -> QVariant {
            if (!message) return QVariant();
            return QString::fromStdString(message->name());
        };
        const auto &flag_id = flag(_protobuf);
        if (flag_id == kMission) {
            return getUserName(static_cast<pb::mission::Mission *>(_protobuf));
        } else if (flag_id == kCollection) {
            return getUserName(static_cast<pb::mission::Mission::Collection *>(_protobuf));
        } else if (flag_id == kPoint) {
            return getUserName(static_cast<pb::mission::Mission::Element::Point *>(_protobuf));
        } else if (flag_id == kRail) {
            return getUserName(static_cast<pb::mission::Mission::Element::Rail *>(_protobuf));
        } else if (flag_id == kSegment) {
            return getUserName(static_cast<pb::mission::Mission::Element::Segment *>(_protobuf));
        } else {
            qCWarning(LC_RMI) << "fail getting data, missing flag [" << flag_id << "]";
        }
    }

    else if (role == Qt::UserRolePack) {
        // return QVariant::fromValue(MissionItem::wrap(_protobuf));
    }

    return QVariant();
}

// Returns the displayed icon of the item. The icon is figured out by looking at
// the flag and flag collection identifier.
QVariant MissionItem::icon() const
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
        qCWarning(LC_RMI) << "fail getting icon, missing flag [" << flag_id << "]";
        return QVariant();
    }
}

// Returns the flag collection identifier of the item. The flag collection
// identifier is figured out by looking at the item children flag identifier
//  - A Route is a collection of Point.
//  - A Family is a collection of Rail.
MissionItem::Flag MissionItem::collectionFlag() const
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

// Sets the data from the specified flag identifier this also create all dependant
// children.
void MissionItem::setDataFromFlag(const MissionItem::Flag new_flag)
{
    if (!_parent->isFlagSupported(new_flag)) {
        qCWarning(LC_RMI) << "fail setting data, flag not supported [" << new_flag << "]";
        return;
    }

    const auto &parent_flag_id = flag(_parent->protobuf());
    const auto &parent_count_child = _parent->countChild() - 1;

    if (new_flag == kMission) {
        auto *protobuf = static_cast<pb::mission::Mission *>(_protobuf);
        if (protobuf) {
            protobuf->set_name(QObject::tr("My Amazing Mission").toStdString());
        } else {
            qCWarning(LC_RMI) << "fail setting data, null protobuf pointer";
        }

    } else if (new_flag == kCollection) {
        auto *protobuf = static_cast<pb::mission::Mission *>(addCollectionProtobuf(parent_flag_id, _parent->_protobuf));
        if (protobuf) {
            protobuf->set_name(QObject::tr("Collection %1").arg(parent_count_child).toStdString());
            _protobuf = protobuf;
        } else {
            qCWarning(LC_RMI) << "fail setting data, action not allowed for [" << new_flag << "]";
        }

    } else if (new_flag == kPoint) {
        auto *protobuf =
            static_cast<pb::mission::Mission::Element::Point *>(addPointProtobuf(parent_flag_id, _parent->_protobuf));
        if (protobuf) {
            protobuf->set_name(QObject::tr("Point %1").arg(parent_count_child).toStdString());
            _protobuf = protobuf;
        } else {
            qCWarning(LC_RMI) << "fail setting data, action not allowed for [" << new_flag << "]";
        }

    } else if (new_flag == kRail) {
        auto *protobuf =
            static_cast<pb::mission::Mission::Element::Rail *>(addRailProtobuf(parent_flag_id, _parent->_protobuf));
        if (protobuf) {
            protobuf->set_name(QObject::tr("Rail %1").arg(parent_count_child).toStdString());
            protobuf->mutable_p0()->set_name("JA");
            protobuf->mutable_p1()->set_name("JB");
            _protobuf = protobuf;
            addChild(protobuf->mutable_p0(), this);
            addChild(protobuf->mutable_p1(), this);
        } else {
            qCWarning(LC_RMI) << "fail setting data, action not allowed for [" << new_flag << "]";
        }

    } else if (new_flag == kSegment) {
        auto *protobuf = static_cast<pb::mission::Mission::Element::Segment *>(
            addSegmentProtobuf(parent_flag_id, _parent->_protobuf));
        if (protobuf) {
            protobuf->set_name(QObject::tr("Segment %1").arg(parent_count_child).toStdString());
            protobuf->mutable_p0()->set_name("SA");
            protobuf->mutable_p1()->set_name("SB");
            _protobuf = protobuf;
            addChild(protobuf->mutable_p0(), this);
            addChild(protobuf->mutable_p1(), this);
        } else {
            qCWarning(LC_RMI) << "fail setting data, action not allowed for [" << new_flag << "]";
        }

    } else {
        qCWarning(LC_RMI) << "fail setting data, mission flag [" << new_flag << "]";
    }
}

// Sets the data from the specified protobuf message this also expands the protobuf
// message in order to create all children.
bool MissionItem::setDataFromProtobuf(Protobuf *protobuf)
{
    if (!protobuf) {
        qCWarning(LC_RMI) << "fail setting data, null protobuf pointer";
        return false;
    }

    setProtobuf(protobuf);

    const auto &flag_id = flag(_protobuf);

    if (flag_id == kMission) {
        expandMissionProtobuf(static_cast<pb::mission::Mission *>(_protobuf), this);

    } else if (flag_id == kCollection) {
        expandCollectionProtobuf(static_cast<pb::mission::Mission::Collection *>(_protobuf), this);

    } else if (flag_id == kElement) {
        expandElementProtobuf(static_cast<pb::mission::Mission::Element *>(_protobuf), this);

    } else if (flag_id == kPoint) {
        expandPointProtobuf(static_cast<pb::mission::Mission::Element::Point *>(_protobuf), this);

    } else if (flag_id == kRail) {
        expandRailProtobuf(static_cast<pb::mission::Mission::Element::Rail *>(_protobuf), this);

    } else if (flag_id == kSegment) {
        expandSegmentProtobuf(static_cast<pb::mission::Mission::Element::Segment *>(_protobuf), this);

    } else {
        qCWarning(LC_RMI) << "fail setting data, mission flag [" << flag_id << "]";
        return false;
    }

    return true;
}

// Sets the data from the specified protobuf message this also expands the protobuf
// message in order to create all children.
bool MissionItem::setDataFromProtobuf(const QByteArray stream)
{
    //    if (!protobuf) {
    //        qWarning() << CLASSNAME << "[Warning] setting protobuf and expanding, null protobuf pointer";
    //        return false;
    //    }

    //    setProtobuf(protobuf);

    //    const auto &flag_id = flag(_protobuf);

    //    if (flag_id == kMission) {
    //        expandMissionProtobuf(static_cast<pb::mission::Mission *>(_protobuf), this);

    //    } else if (flag_id == kCollection) {
    //        expandCollectionProtobuf(static_cast<pb::mission::Mission::Collection *>(_protobuf), this);

    //    } else if (flag_id == kElement) {
    //        expandElementProtobuf(static_cast<pb::mission::Mission::Element *>(_protobuf), this);

    //    } else if (flag_id == kPoint) {
    //        expandPointProtobuf(static_cast<pb::mission::Mission::Element::Point *>(_protobuf), this);

    //    } else if (flag_id == kRail) {
    //        expandRailProtobuf(static_cast<pb::mission::Mission::Element::Rail *>(_protobuf), this);

    //    } else if (flag_id == kSegment) {
    //        expandSegmentProtobuf(static_cast<pb::mission::Mission::Element::Segment *>(_protobuf), this);

    //    } else {
    //        qWarning() << CLASSNAME << "[Warning] fail setting protobuf and expanding, missing flag" << flag_id;
    //        return false;
    //    }

    //    return true;
}
