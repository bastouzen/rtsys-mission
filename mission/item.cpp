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

const auto ProtobufMission = rtsys::mission::Mission::descriptor() -> name();
const auto ProtobufDevice = rtsys::mission::Device::descriptor() -> name();
const auto ProtobufCollection = rtsys::mission::Collection::descriptor() -> name();
const auto ProtobufBlock = rtsys::mission::Block::descriptor() -> name();
const auto ProtobufPoint = rtsys::mission::Block::Point::descriptor() -> name();
const auto ProtobufLine = rtsys::mission::Block::Line::descriptor() -> name();

// ===
// === Function
// ============================================================================ //

// Returns the component specified by the underlying protobuf message.
MissionItem::Feature MissionItem::component(const Protobuf *protobuf)
{
    if (!protobuf) return Feature::kUndefined;

    const auto &name = protobuf->GetDescriptor()->name();

    if (name == ProtobufMission) {
        return Feature::kMission;
    } else if (name == ProtobufDevice) {
        return Feature::kDevice;
    } else if (name == ProtobufCollection) {
        return Feature::kCollection;
    } else if (name == ProtobufPoint) {
        return Feature::kPoint;
    } else if (name == ProtobufLine) {
        const auto &type = static_cast<const rtsys::mission::Block::Line *>(protobuf)->type();
        if (type == rtsys::mission::Block::Line::LINE_RAIL) {
            return Feature::kRail;
        } else {
            return Feature::kSegment;
        }
    } else {
        qCWarning(LC_RMI) << "fail getting component, missing message [" << name.data() << "]";
        return Feature::kUndefined;
    }
}

// Serializes the underlying protobuf message.
QByteArray MissionItem::pack(const Protobuf &protobuf)
{
    return QByteArray::fromStdString(protobuf.SerializeAsString());
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

// Returns the supported feature. This depends on the item component and parent
// item component.
unsigned int MissionItem::supportedFeatures() const
{
    const auto &component_ = component(_protobuf);

    if (component_ == kUndefined) {
        return Bit(kMission);

    } else if (component_ == kMission) {
        return Bit(kDelete) | Bit(kEdit) | /* Add */ Bit(kDevice) | Bit(kCollection) | Bit(kPoint) | Bit(kRail) |
               Bit(kSegment);

    } else if (component_ == kDevice) {
        return Bit(kDelete) | Bit(kEdit) | /* Add */ Bit(kCollection) | Bit(kPoint) | Bit(kRail) | Bit(kSegment);

    } else if (component_ == kCollection) {
        const auto &collection_ = collection();
        if (collection_ == kFamily || collection_ == kRoute) {
            return Bit(kDelete) | Bit(kEdit) /*| Bit(kSwap)*/ | /* Add */ Bit(kPoint) | Bit(kRail) | Bit(kSegment);
        } else {
            return Bit(kDelete) | Bit(kEdit) | /* Add */ Bit(kPoint) | Bit(kRail) | Bit(kSegment);
        }

    } else if (component_ == kRail || component_ == kSegment) {
        return Bit(kDelete) | Bit(kEdit) | Bit(kSwap);

    } else if (component_ == kPoint) {
        const auto &component_parent = component(_parent->_protobuf);
        if (component_parent == kRail || component_parent == kSegment) {
            return Bit(kEdit);
        }
        return Bit(kDelete) | Bit(kEdit);

    } else {
        qCWarning(LC_RMI) << "fail getting supported features, missing component [" << component_ << "]";
    }

    return 0;
}

// Returns the supported flag. This depends on the item component and parent
// item component.
Qt::ItemFlags MissionItem::supportedFlags() const
{
    const auto &component_ = component(_protobuf);

    if (component_ == kMission) {
        return Qt::ItemIsDropEnabled;

    } else if (component_ == kDevice) {
        return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;

    } else if (component_ == kCollection) {
        return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;

    } else if (component_ == kRail || component_ == kSegment) {
        return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;

    } else if (component_ == kPoint) {
        const auto &component_parent = component(_parent->_protobuf);
        if (component_parent == kRail || component_parent == kSegment) {
            return Qt::NoItemFlags;
        }
        return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;

    } else {
        return Qt::NoItemFlags;
    }
}

// Removes the child item specified by the given row.
void MissionItem::removeChild(int row)
{
    Q_ASSERT(!(row < 0 || row >= _childs.size()));

    auto removeProtobuf = [&]() {
        // Remove the child protobuf message.
        const auto &component_ = component(_protobuf);
        if (component_ == kMission) {
            RepeatedFieldRemoveAt(row, static_cast<rtsys::mission::Mission *>(_protobuf)->mutable_components());

        } else if (component_ == kDevice) {
            RepeatedFieldRemoveAt(row, static_cast<rtsys::mission::Device *>(_protobuf)->mutable_components());

        } else if (component_ == kCollection) {
            RepeatedFieldRemoveAt(row, static_cast<rtsys::mission::Collection *>(_protobuf)->mutable_blocks());

        } else if (component_ == kUndefined) {
            // In this case we want to remove a top-level row-protobuf message, it
            // means this backend is the one linked to the root item. In order to
            // remove the row-protobuf message, we first retrieve the child item
            // specified by the row and then clear the underlying protobuf message.
            if (countChild() >= row) {
                child(row)->_protobuf->Clear();
            } else {
                qCWarning(LC_RMI) << "fail removing child, missing row [" << row << "]";
                return false;
            }

        } else if (component_ == kRail || component_ == kSegment) {
            // Do nothing !!

        } else {
            qCWarning(LC_RMI) << "fail removing child, missing component [" << component_ << "]";
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
    if (role == Qt::EditRole) {
        // Here we use some king to inline template function thank to C++14
        // "lambda function with auto parameter"
        auto setUserRoleName = [&](auto *protobuf) { protobuf->set_name(value.toString().toStdString()); };

        const auto &component_ = component(_protobuf);
        if (component_ == kMission) {
            setUserRoleName(static_cast<rtsys::mission::Mission *>(_protobuf));
        } else if (component_ == kDevice) {
            setUserRoleName(static_cast<rtsys::mission::Device *>(_protobuf));
        } else if (component_ == kCollection) {
            setUserRoleName(static_cast<rtsys::mission::Collection *>(_protobuf));
        } else if (component_ == kPoint) {
            setUserRoleName(static_cast<rtsys::mission::Block::Point *>(_protobuf));
        } else if (component_ == kRail || component_ == kSegment) {
            setUserRoleName(static_cast<rtsys::mission::Block::Line *>(_protobuf));
        } else {
            qCWarning(LC_RMI) << "fail setting data, missing component [" << component_ << "]";
            return false;
        }
        return true;

    } else if (role == Qt::UserRoleComponent) {
        setDataFromComponent(value.value<MissionItem::Feature>());
        return true;

    } else if (role == Qt::UserRolePack) {
        setDataFromProtobuf(value.toByteArray());
        return true;

    } else if (role == Qt::UserRoleWrapper) {
        _protobuf = value.value<MissionItem::Wrapper>().pointer;
        return true;
    }

    return false;
}

// Returns the data specified by the role.
QVariant MissionItem::data(const int role, const int column) const
{
    if (role == Qt::DecorationRole && column == 0) {
        return icon();

    } else if (role == Qt::DisplayRole && column == 0) {
        // Use reflection property of Qt for getting the name of the component.
        return QString(QMetaEnum::fromType<Feature>().valueToKey(component(_protobuf))).mid(1, -1);

    } else if ((role == Qt::DisplayRole && column == 1) || role == Qt::EditRole) {
        // Here we use some king to inline template function thank to C++14
        // "lambda function with auto parameter"
        auto getUserName = [&](auto *message) -> QVariant {
            if (!message) return QVariant();
            return QString::fromStdString(message->name());
        };
        const auto &component_ = component(_protobuf);
        if (component_ == kMission) {
            return getUserName(static_cast<rtsys::mission::Mission *>(_protobuf));
        } else if (component_ == kDevice) {
            return getUserName(static_cast<rtsys::mission::Device *>(_protobuf));
        } else if (component_ == kCollection) {
            return getUserName(static_cast<rtsys::mission::Collection *>(_protobuf));
        } else if (component_ == kPoint) {
            return getUserName(static_cast<rtsys::mission::Block::Point *>(_protobuf));
        } else if (component_ == kRail || component_ == kSegment) {
            return getUserName(static_cast<rtsys::mission::Block::Line *>(_protobuf));
        } else {
            qCWarning(LC_RMI) << "fail getting data, missing component [" << component_ << "]";
        }

    } else if (role == Qt::UserRoleComponent) {
        return component(_protobuf);

    } else if (role == Qt::UserRolePack) {
        return MissionItem::pack(*_protobuf);

    } else if (role == Qt::UserRoleWrapper) {
        return QVariant::fromValue(MissionItem::wrap(_protobuf));
    }

    return QVariant();
}

// Returns the displayed icon of the item. The icon is figured out by looking at
// the component and component collection.
QVariant MissionItem::icon() const
{
    const auto &component_ = component(_protobuf);

    if (component_ == kMission) {
        return QIcon(ResourceIconMission);
    } else if (component_ == kDevice) {
        return QIcon(ResourceIconCollection);
    } else if (component_ == kCollection) {
        const auto &collection_ = collection();
        if (collection_ == kRoute) {
            return QIcon(ResourceIconRoute);
        } else if (collection_ == kFamily) {
            return QIcon(ResourceIconFamily);
        } else {
            return QIcon(ResourceIconCollection);
        }
    } else if (component_ == kRail) {
        return QIcon(ResourceIconRail);
    } else if (component_ == kSegment) {
        return QIcon(ResourceIconSegment);
    } else if (component_ == kPoint) {
        const auto &component_parnent = component(_parent->_protobuf);
        if (component_parnent == kRail) {
            return QIcon(ResourceIconRailPoint);
        } else if (component_parnent == kSegment) {
            return QIcon(ResourceIconRailPoint);
        } else {
            return QIcon(ResourceIconPoint);
        }
    } else {
        qCWarning(LC_RMI) << "fail getting icon, missing component [" << component_ << "]";
        return QVariant();
    }
}

// Returns the component collection of the item. The component collection
// is figured out by looking at the item children component
//  - A Route is a collection of Point.
//  - A Family is a collection of Rail.
MissionItem::Feature MissionItem::collection() const
{
    if (countChild()) {
        auto is_route = true;
        auto is_family = true;
        for (auto *child_item : _childs) {
            const auto &component_ = component(child_item->_protobuf);
            is_route &= component_ == kPoint;
            is_family &= component_ == kRail;
        }
        if (is_route) {
            return kRoute;
        } else if (is_family) {
            return kFamily;
        }
    }
    return kScenario;
}

// Sets the data from the specified component this also create all dependant
// children.
void MissionItem::setDataFromComponent(const MissionItem::Feature component)
{
    const auto &enumerate = _parent->countChild() - 1;
    if (component == kMission) {
        auto *protobuf = static_cast<rtsys::mission::Mission *>(_protobuf);
        protobuf->set_name(QObject::tr("New Mission").toStdString());

    } else if (component == kDevice) {
        auto *protobuf = static_cast<rtsys::mission::Device *>(insertDeviceProtobuf(row(), _parent));
        protobuf->set_name(QObject::tr("Device %1").arg(enumerate).toStdString());
        _protobuf = protobuf;

    } else if (component == kCollection) {
        auto *protobuf = static_cast<rtsys::mission::Mission *>(insertCollectionProtobuf(row(), _parent));
        protobuf->set_name(QObject::tr("Collection %1").arg(enumerate).toStdString());
        _protobuf = protobuf;

    } else if (component == kPoint) {
        auto *protobuf = static_cast<rtsys::mission::Block::Point *>(insertPointProtobuf(row(), _parent));
        protobuf->set_name(QObject::tr("Point %1").arg(enumerate).toStdString());
        _protobuf = protobuf;

    } else if (component == kRail || component == kSegment) {
        auto *protobuf = static_cast<rtsys::mission::Block::Line *>(insertLineProtobuf(row(), _parent));
        if (component == kRail) {
            protobuf->set_type(rtsys::mission::Block::Line::LINE_RAIL);
            protobuf->set_name(QObject::tr("Rail %1").arg(enumerate).toStdString());
            protobuf->add_points()->set_name("JA");
            protobuf->add_points()->set_name("JB");
        } else if (component == kSegment) {
            protobuf->set_type(rtsys::mission::Block::Line::LINE_SEGMENT);
            protobuf->set_name(QObject::tr("Segment %1").arg(enumerate).toStdString());
            protobuf->add_points()->set_name("SA");
            protobuf->add_points()->set_name("SB");
        }
        _protobuf = protobuf;
        addChild(protobuf->mutable_points(0), this);
        addChild(protobuf->mutable_points(1), this);

    } else {
        qCWarning(LC_RMI) << "fail setting data, missing component [" << component << "]";
    }
}

// Sets the data from the specified protobuf message this also expands the protobuf
// message in order to create all children.
bool MissionItem::setDataFromProtobuf(const QByteArray &packed)
{
    const auto &component_ = component(_protobuf);

    // We remove the already created children of Rail and Segment. These children
    // were created by calling "setDataFromComponent" but as we use the parse
    // function from google protobuf, the nested field messages may be
    // desallocated and reallocated by the parsing process. So in order to avoid
    // dangling pointer we remove all the children.
    if (component_ == kRail || component_ == kSegment) {
        while (countChild()) removeChild(0);
    }

    if (component_ == kMission) {
        if (_protobuf->ParseFromString(packed.toStdString())) {
            expandMissionProtobuf(static_cast<rtsys::mission::Mission *>(_protobuf), this);
            return true;
        }

    } else if (component_ == kDevice) {
        if (_protobuf->ParseFromString(packed.toStdString())) {
            expandDeviceProtobuf(static_cast<rtsys::mission::Device *>(_protobuf), this);
            return true;
        }

    } else if (component_ == kCollection) {
        if (_protobuf->ParseFromString(packed.toStdString())) {
            expandCollectionProtobuf(static_cast<rtsys::mission::Collection *>(_protobuf), this);
            return true;
        }

    } else if (component_ == kPoint) {
        if (_protobuf->ParseFromString(packed.toStdString())) {
            return true;
        }

    } else if (component_ == kRail || component_ == kSegment) {
        if (_protobuf->ParseFromString(packed.toStdString())) {
            expandLineProtobuf(static_cast<rtsys::mission::Block::Line *>(_protobuf), this);
            return true;
        }

    } else {
        qCWarning(LC_RMI) << "fail setting data, missing component [" << component_ << "]";
    }

    return false;
}
