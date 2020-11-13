// ===
// === Include
// ============================================================================ //

#include "mission/item.h"
#include "protobuf/mission.pb.h"

#include <QDebug>
#include <QIcon>

// ===
// === Function
// ============================================================================ //

// Creates then appends an item into the parent children with the specified
// underlying protobuf message.
MissionItem *appendRowItem(google::protobuf::Message *msg, MissionItem *parent)
{
    auto item = new MissionItem(MissionBackend::data(msg), msg, parent);
    parent->appendChild(item);
    qInfo() << "MissionItem :: succeed appending item" << item->data(1).toString();
    return item;
}

// Creates then appends an element into the parent children with the specified
// underlying protobuf message.
MissionItem *appendRowElement(pb::mission::Mission::Element *element, MissionItem *parent)
{
    MissionItem *item = nullptr;
    switch (element->element_case()) {
        case pb::mission::Mission::Element::kPoint:
            item = appendRowItem(element->mutable_point(), parent);
            break;
        case pb::mission::Mission::Element::kRail:
            item = appendRowItem(element->mutable_rail(), parent);
            appendRowItem(element->mutable_rail()->mutable_p0(), item);
            appendRowItem(element->mutable_rail()->mutable_p1(), item);
            break;
        case pb::mission::Mission::Element::kSegment:
            item = appendRowItem(element->mutable_segment(), parent);
            appendRowItem(element->mutable_segment()->mutable_p0(), item);
            appendRowItem(element->mutable_segment()->mutable_p1(), item);
            break;
        default:
            qWarning() << "MissionItem :: [Warning] in appendRowElement()";
            break;
    }
    return item;
};

// Creates then appends a collection into the parent children with the specified
// underlying protobuf message.
MissionItem *appendRowCollection(pb::mission::Mission::Collection *collection, MissionItem *parent)
{
    auto item = appendRowItem(collection, parent);
    for (auto &element : *collection->mutable_elements()) {
        appendRowElement(&element, item);
    }
    return item;
};

// Creates then appends a mission into the parent children with the specified
// underlying protobuf message.
MissionItem *appendRowMission(pb::mission::Mission *mission, MissionItem *parent)
{
    auto item = appendRowItem(mission, parent);
    for (auto &component : *mission->mutable_components()) {
        switch (component.component_case()) {
            case pb::mission::Mission::Component::kElement:
                appendRowElement(component.mutable_element(), item);
                break;
            case pb::mission::Mission::Component::kCollection:
                appendRowCollection(component.mutable_collection(), item);
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

// Returns the child specified by the given row.
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

// Creates then appends a child into the parent children with the specified
// underlying protobuf message.
void MissionItem::appendRow(google::protobuf::Message *protobuf)
{
    if (!protobuf) return;

    const auto &component = MissionBackend::componentType(protobuf);
    if (component == MissionBackend::kMission) {
        appendRowMission(static_cast<pb::mission::Mission *>(protobuf), this);
    } else if (component == MissionBackend::kCollection) {
        appendRowCollection(static_cast<pb::mission::Mission::Collection *>(protobuf), this);
    } else {
        appendRowElement(static_cast<pb::mission::Mission::Element *>(protobuf), this);
    }
}

void MissionItem::appendRow(const MissionBackend::Action action)
{
    appendRow(_backend.append(action));
}

// Removes the child specified by the given row. This also removes the
// underlying protobuf data through the backend.
void MissionItem::removeRow(int row)
{
    if (row < 0 || row >= _childs.size()) return;

    _backend.remove(row);
    auto *pointer = child(row);
    _childs.remove(row);
    delete pointer;
    pointer = nullptr;
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
