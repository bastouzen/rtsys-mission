// ===
// === Include
// ============================================================================ //

#include "mission/mission_model_item.h"
#include "protobuf/mission.pb.h"

#include <QDebug>
#include <QIcon>

#define CastToItem(index) static_cast<MissionItem *>(index.internalPointer())

// ===
// === Class
// ============================================================================ //

MissionItem::MissionItem(const QVector<QVariant> &data, google::protobuf::Message *protobuf, MissionItem *parent)
    : _data(data)
    , _parent(parent)
    , _backend(protobuf, this)
{
}

MissionItem::~MissionItem()
{
    qDeleteAll(_childs);
}

void MissionItem::appendRow(google::protobuf::Message *protobuf)
{

    auto _appendRowItem = [&](google::protobuf::Message *msg, MissionItem *parent) {
        auto item = new MissionItem(MissionBackend::data(msg), msg, parent);
        parent->appendChild(item);
        qInfo() << "MissionItem :: succeed appending row" << item->data(1).toString();
        return item;
    };

    auto _appendRowElement = [&](pb::mission::Mission::Element *element, MissionItem *parent) {
        MissionItem *item;
        switch (element->element_case()) {
            case pb::mission::Mission::Element::kPoint:
                item = _appendRowItem(element->mutable_point(), parent);
                break;
            case pb::mission::Mission::Element::kRail:
                item = _appendRowItem(element->mutable_rail(), parent);
                _appendRowItem(element->mutable_rail()->mutable_p0(), item);
                _appendRowItem(element->mutable_rail()->mutable_p1(), item);
                break;
            case pb::mission::Mission::Element::kSegment:
                item = _appendRowItem(element->mutable_segment(), parent);
                _appendRowItem(element->mutable_segment()->mutable_p0(), item);
                _appendRowItem(element->mutable_segment()->mutable_p1(), item);
                break;
            default:
                break;
        }
    };

    auto _appendRowCollection = [&](pb::mission::Mission::Collection *collection, MissionItem *parent) {
        auto item = _appendRowItem(collection, parent);
        for (auto &element : *collection->mutable_elements()) {
            _appendRowElement(&element, item);
        }
    };

    auto _appendRowMission = [&](pb::mission::Mission *mission, MissionItem *parent) {
        auto item = _appendRowItem(mission, parent);
        for (auto &component : *mission->mutable_components()) {
            switch (component.component_case()) {
                case pb::mission::Mission::Component::kElement:
                    _appendRowElement(component.mutable_element(), item);
                    break;
                case pb::mission::Mission::Component::kCollection:
                    _appendRowCollection(component.mutable_collection(), item);
                    break;
                default:
                    break;
            }
        }
    };

    if (protobuf) {
        const auto &component = MissionBackend::componentType(protobuf);

        if (component == MissionBackend::kMission)
            _appendRowMission(static_cast<pb::mission::Mission *>(protobuf), this);

        if (component == MissionBackend::kCollection)
            _appendRowCollection(static_cast<pb::mission::Mission::Collection *>(protobuf), this);

        if (component == MissionBackend::kPoint)
            _appendRowItem(static_cast<pb::mission::Mission::Element::Point *>(protobuf), this);

        if (component == MissionBackend::kRail)
            _appendRowItem(static_cast<pb::mission::Mission::Element::Rail *>(protobuf), this);

        if (component == MissionBackend::kSegment)
            _appendRowItem(static_cast<pb::mission::Mission::Element::Segment *>(protobuf), this);

    } else {
        qWarning() << "MissionItem :: fail appending row, the pointer protobuf message is null";
    }
}

// void MissionItem::addMission(google::protobuf::Message *protobuf)
//{
////    if (protobuf) {
////        auto item = createItem(this, static_cast<pb::mission::Mission *>(protobuf));
////        appendChild(item);
////        qWarning() << "MissionItem :: adding existing mission" << _childs.last()->data(1).toString() << "succeed.";
////    } else {
////        auto item = _backend.addMission
////        appendChild()
////    }
//}

//// Adds a point under the specified parent index. This check if the parent is
//// valid and if the "addPoint" action is enabled for the specified parent index.
// void MissionItem::addPoint()
//{
//    if (_backend.hasEnableAction(MissionBackend::Action::kAddPoint)) {
//        auto *protobuf = static_cast<pb::mission::Mission::Element::Point *>(_backend.addPoint());
//        _childs.append(new MissionItem(
//            {QString::fromStdString(protobuf->GetDescriptor()->name()), QString::fromStdString(protobuf->name())},
//            protobuf, this));
//        qWarning() << "MissionItem" << __func__ << "adding point succeed";

//    } else {
//        qWarning() << "MissionItem" << __func__ << "adding point fail because action is not enabled";
//    }
//}

// Removes the child specified by the given row. This also removes the
// underlying protobuf data through the backend.
// void MissionItem::removeChild(int row)
//{
//    if (row < 0 || row >= _childs.size()) return;

//    _backend.remove(row);
//    auto *pointer = child(row);
//    _childs.remove(row);
//    delete pointer;
//    pointer = nullptr;
//}

// Return the child specified by the given row.
MissionItem *MissionItem::child(int row)
{
    if (row < 0 || row >= _childs.size()) return nullptr;

    return _childs.at(row);
}

// Returns the data specified by the column.
QVariant MissionItem::data(int column) const
{
    if (column < 0 || column >= _data.size()) {
        return QVariant();
    }
    return _data.at(column);
}

// Returns the number of rows of the item, it means that row is returning
// the number of its children.
int MissionItem::row() const
{
    if (_parent) {
        return _parent->_childs.indexOf(const_cast<MissionItem *>(this));
    }
    return 0;
}
