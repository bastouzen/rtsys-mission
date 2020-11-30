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

// Returns the displayed icon of the item. The icon is figured out by looking at
// the item feature and item collection feature.
QVariant MissionItem::icon() const
{
    const auto &feature = this->feature();

    if (feature & kMission) {
        return QIcon(ResourceIconMission);

    } else if (feature & kDevice) {
        return QIcon(ResourceIconCollection);

    } else if (feature & kCollection) {
        if (feature & kRoute) {
            return QIcon(ResourceIconRoute);
        } else if (feature & kFamily) {
            return QIcon(ResourceIconFamily);
        } else {
            return QIcon(ResourceIconCollection);
        }

    } else if (feature & kLine) {
        if (feature & kRail) {
            return QIcon(ResourceIconRail);
        } else if (feature & kSegment) {
            return QIcon(ResourceIconSegment);
        } else {
            return QIcon(ResourceIconCollection);
        }

    } else if (feature & kPoint) {
        if (_parent->feature() & kLine) {
            return QIcon(ResourceIconRailPoint);
        } else {
            return QIcon(ResourceIconPoint);
        }
    } else {
        qCWarning(LC_RMI) << "fail getting icon, missing feature [" << feature << "]";
        return QVariant();
    }
}

// Returns the feature collection of the item. The feature collection
// is figured out by looking at the item children features
//  - A Route is a collection of Point.
//  - A Family is a collection of Rail.
MissionItem::Features MissionItem::collection() const
{
    if (countChild()) {
        auto is_route = true;
        auto is_family = true;
        for (const auto *child_item : _childs) {
            const auto &features = child_item->feature();
            is_route &= features.testFlag(kPoint);
            is_family &= features.testFlag(kRail);
        }
        if (is_route) {
            return kRoute;
        } else if (is_family) {
            return kFamily;
        }
    }
    return kScenario;
}

// Returns the feature specified by the protobuf message.
MissionItem::Features MissionItem::feature() const
{
    if (!_protobuf) return kUndefined;

    const auto &name = _protobuf->GetDescriptor()->name();

    if (name == ProtobufMission) {
        return kMission;

    } else if (name == ProtobufDevice) {
        return kDevice;

    } else if (name == ProtobufCollection) {
        return kCollection | this->collection();

    } else if (name == ProtobufLine) {
        const auto &type = static_cast<const rtsys::mission::Block::Line *>(_protobuf)->type();
        if (type == rtsys::mission::Block::Line::LINE_RAIL) {
            return kLine | kRail;
        } else {
            return kLine | kSegment;
        }

    } else if (name == ProtobufPoint) {
        return kPoint;

    } else {
        qCWarning(LC_RMI) << "fail getting feature, missing name [" << name.data() << "]";
        return kUndefined;
    }
}

// Returns the supported features of the item, depending on its underlying protobuf
// message and parent item underlying protobuf message.
MissionItem::Features MissionItem::supportedFeatures() const
{
    const auto feature = this->feature();

    if (!feature /*feature == kUndefined*/) {
        return kMission;

    } else if (feature & kMission) {
        return kDelete | kEdit | /* Add */ kDevice | kCollection | kLine | kPoint;

    } else if (feature & kDevice) {
        return kDelete | kEdit | /* Add */ kCollection | kLine | kPoint;

    } else if (feature & kCollection) {
        if (collection() & (kFamily | kRoute)) {
            return kDelete | kEdit | kSwap | /* Add */ kLine | kPoint;
        } else {
            return kDelete | kEdit | /* Add */ kLine | kPoint;
        }

    } else if (feature & kLine) {
        return kDelete | kEdit | kSwap;

    } else if (feature & kPoint) {
        if (_parent->feature() & kLine) {
            return kEdit;
        }
        return kDelete | kEdit;

    } else {
        qCWarning(LC_RMI) << "fail getting supported features, missing feature [" << feature << "]";
        return MissionItem::Features();
    }
}

// Returns the supported drop flag. This depends on item underlying protobuf
// message and parent item underlying protobuf message.
Qt::ItemFlags MissionItem::supportedDropFlags() const
{
    const auto feature = this->feature();

    if (feature & kMission) {
        return Qt::ItemIsDropEnabled;

    } else if (feature & kDevice) {
        return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;

    } else if (feature & kCollection) {
        return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;

    } else if (feature & kLine) {
        return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;

    } else if (feature & kPoint) {
        if (_parent->feature() & kLine) {
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
        const auto &feature = this->feature();

        if (!feature /*feature == kUndefined*/) {
            // In this case we want to remove a top-level child item, so that
            // child item's parent is linked to the root item. In order to
            // remove this top-level child item, we first retrieve the child
            // item specified by the row and then clear the underlying
            // protobuf message.
            if (countChild() >= row) {
                child(row)->_protobuf->Clear();
                return true;
            } else {
                qCWarning(LC_RMI) << "fail removing child, missing row [" << row << "]";
                return false;
            }

        } else if (feature & kMission) {
            RepeatedFieldRemoveAt(row, static_cast<rtsys::mission::Mission *>(_protobuf)->mutable_components());
            return true;

        } else if (feature & kDevice) {
            RepeatedFieldRemoveAt(row, static_cast<rtsys::mission::Device *>(_protobuf)->mutable_components());
            return true;

        } else if (feature & kCollection) {
            RepeatedFieldRemoveAt(row, static_cast<rtsys::mission::Collection *>(_protobuf)->mutable_blocks());
            return true;

        } else if (feature & (kLine | kPoint)) {
            return false;

        } else {
            qCWarning(LC_RMI) << "fail removing child, missing feature [" << feature << "]";
            return false;
        }
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

// Returns the data specified by the role.
QVariant MissionItem::data(const int role, const int column) const
{
    if (role == Qt::DecorationRole && column == 0) {
        return icon();

    } else if (role == Qt::DisplayRole && column == 0) {
        return QString(QMetaEnum::fromType<FeatureFlag>().valueToKey(feature() & ~(kLine | kCollection))).mid(1, -1);

    } else if ((role == Qt::DisplayRole && column == 1) || role == Qt::EditRole) {
        // Here we use some king to inline template function thank to C++14
        // "lambda function with auto parameter"
        auto getUserName = [&](auto *message) -> QVariant {
            if (!message) return QVariant();
            return QString::fromStdString(message->name());
        };

        const auto &feature = this->feature();

        if (feature & kMission) {
            return getUserName(static_cast<rtsys::mission::Mission *>(_protobuf));

        } else if (feature & kDevice) {
            return getUserName(static_cast<rtsys::mission::Device *>(_protobuf));

        } else if (feature & kCollection) {
            return getUserName(static_cast<rtsys::mission::Collection *>(_protobuf));

        } else if (feature & kLine) {
            return getUserName(static_cast<rtsys::mission::Block::Line *>(_protobuf));

        } else if (feature & kPoint) {
            return getUserName(static_cast<rtsys::mission::Block::Point *>(_protobuf));

        } else {
            qCWarning(LC_RMI) << "fail getting data, missing feature [" << feature << "]";
        }

    } else if (role == Qt::UserRoleFeature) {
        return Features::Int(feature());

    } else if (role == Qt::UserRolePack) {
        return MissionItem::pack(*_protobuf);

    } else if (role == Qt::UserRoleWrapper) {
        return QVariant::fromValue(MissionItem::wrap(_protobuf));
    }

    return QVariant();
}

// Sets the data for the specified value and role. Returns true if successful
// otherwise returns false.
bool MissionItem::setData(const QVariant &value, int role)
{
    if (role == Qt::EditRole) {
        // Here we use some king to inline template function thank to C++14
        // "lambda function with auto parameter"
        auto setUserRoleName = [&](auto *protobuf) { protobuf->set_name(value.toString().toStdString()); };

        const auto &feature = this->feature();
        if (feature & kMission) {
            setUserRoleName(static_cast<rtsys::mission::Mission *>(_protobuf));
            return true;

        } else if (feature & kDevice) {
            setUserRoleName(static_cast<rtsys::mission::Device *>(_protobuf));
            return true;

        } else if (feature & kCollection) {
            setUserRoleName(static_cast<rtsys::mission::Collection *>(_protobuf));
            return true;

        } else if (feature & kLine) {
            setUserRoleName(static_cast<rtsys::mission::Block::Line *>(_protobuf));
            return true;

        } else if (feature & kPoint) {
            setUserRoleName(static_cast<rtsys::mission::Block::Point *>(_protobuf));
            return true;

        } else {
            qCWarning(LC_RMI) << "fail setting data, missing feature [" << feature << "]";
            return false;
        }

    } else if (role == Qt::UserRoleFeature) {
        setDataFromFeature(MissionItem::Features(value.toInt()));
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

// Sets the data from the specified feature this also creates all children.
void MissionItem::setDataFromFeature(const MissionItem::Features feature)
{
    const auto &enumerate = _parent->countChild() - 1;

    qCDebug(LC_RMI) << "setDataFromFeature" << feature;

    if (feature & kMission) {
        auto *protobuf = static_cast<rtsys::mission::Mission *>(_protobuf);
        protobuf->set_name(QObject::tr("My Amazing Mission").toStdString());

    } else if (feature & kDevice) {
        auto *protobuf = static_cast<rtsys::mission::Device *>(insertDeviceProtobuf(row(), _parent));
        protobuf->set_name(QObject::tr("Device %1").arg(enumerate).toStdString());
        _protobuf = protobuf;

    } else if (feature & kCollection) {
        auto *protobuf = static_cast<rtsys::mission::Mission *>(insertCollectionProtobuf(row(), _parent));
        protobuf->set_name(QObject::tr("Collection %1").arg(enumerate).toStdString());
        _protobuf = protobuf;

    } else if (feature & kLine) {
        auto *protobuf = static_cast<rtsys::mission::Block::Line *>(insertLineProtobuf(row(), _parent));
        if (feature & kRail) {
            protobuf->set_type(rtsys::mission::Block::Line::LINE_RAIL);
            protobuf->set_name(QObject::tr("Rail %1").arg(enumerate).toStdString());
            protobuf->add_points()->set_name("JA");
            protobuf->add_points()->set_name("JB");
        } else if (feature & kSegment) {
            protobuf->set_type(rtsys::mission::Block::Line::LINE_SEGMENT);
            protobuf->set_name(QObject::tr("Segment %1").arg(enumerate).toStdString());
            protobuf->add_points()->set_name("SA");
            protobuf->add_points()->set_name("SB");
        }
        _protobuf = protobuf;
        addChild(protobuf->mutable_points(0), this);
        addChild(protobuf->mutable_points(1), this);

    } else if (feature & kPoint) {
        auto *protobuf = static_cast<rtsys::mission::Block::Point *>(insertPointProtobuf(row(), _parent));
        protobuf->set_name(QObject::tr("Point %1").arg(enumerate).toStdString());
        _protobuf = protobuf;

    } else {
        qCWarning(LC_RMI) << "fail setting data, missing feature [" << feature << "]";
    }
}

// Sets the data from the specified protobuf message this also expands the protobuf
// message in order to create all children.
bool MissionItem::setDataFromProtobuf(const QByteArray &packed)
{
    const auto &feature = this->feature();

    // Here we are removing the already created children of Line. These children
    // were created by calling "setDataFromFeature" but as we use the parse
    // function from google protobuf, the nested field messages may be
    // desallocated and reallocated by the parsing process. So in order to avoid
    // dangling pointer we remove all the children.
    if (feature & kLine) {
        while (countChild()) removeChild(0);
    }

    qCDebug(LC_RMI) << "setDataFromProtobuf" << feature;

    if (feature & kMission) {
        if (_protobuf->ParseFromString(packed.toStdString())) {
            expandMissionProtobuf(static_cast<rtsys::mission::Mission *>(_protobuf), this);
            return true;
        }

    } else if (feature & kDevice) {
        if (_protobuf->ParseFromString(packed.toStdString())) {
            expandDeviceProtobuf(static_cast<rtsys::mission::Device *>(_protobuf), this);
            return true;
        }

    } else if (feature & kCollection) {
        if (_protobuf->ParseFromString(packed.toStdString())) {
            expandCollectionProtobuf(static_cast<rtsys::mission::Collection *>(_protobuf), this);
            return true;
        }

    } else if (feature & kLine) {
        if (_protobuf->ParseFromString(packed.toStdString())) {
            expandLineProtobuf(static_cast<rtsys::mission::Block::Line *>(_protobuf), this);
            return true;
        }

    } else if (feature & kPoint) {
        if (_protobuf->ParseFromString(packed.toStdString())) {
            return true;
        }

    } else {
        qCWarning(LC_RMI) << "fail setting data, missing feature [" << feature << "]";
    }

    return false;
}
