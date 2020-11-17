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

// ===
// === Function
// ============================================================================ //

// Creates then appends an item into the parent children with the specified
// underlying protobuf message.
inline ModelItem *appendRowItem(google::protobuf::Message *protobuf, ModelItem *parent)
{
    auto item = new ModelItem(protobuf, parent);
    parent->appendChild(item);
    return item;
}

// Creates then appends an point into the parent children with the specified
// underlying protobuf message.
inline ModelItem *appendRowPoint(pb::mission::Mission::Element::Point *point, ModelItem *parent)
{
    return appendRowItem(point, parent);
}

// Creates then appends an rail into the parent children with the specified
// underlying protobuf message.
inline ModelItem *appendRowRail(pb::mission::Mission::Element::Rail *rail, ModelItem *parent)
{
    return appendRowItem(rail, parent);
}

// Creates then appends an segment into the parent children with the specified
// underlying protobuf message.
inline ModelItem *appendRowSegment(pb::mission::Mission::Element::Segment *segment, ModelItem *parent)
{
    return appendRowItem(segment, parent);
}

// Creates then appends an element into the parent children with the specified
// underlying protobuf message.
inline ModelItem *appendRowElement(pb::mission::Mission::Element *element, ModelItem *parent)
{
    ModelItem *item = nullptr;
    switch (element->element_case()) {
        case pb::mission::Mission::Element::kPoint:
            item = appendRowPoint(element->mutable_point(), parent);
            break;
        case pb::mission::Mission::Element::kRail:
            item = appendRowRail(element->mutable_rail(), parent);
            break;
        case pb::mission::Mission::Element::kSegment:
            item = appendRowSegment(element->mutable_segment(), parent);
            break;
        default:
            qWarning() << CLASSNAME << "[Warning] fail appending row element, missing definition"
                       << element->element_case();
            break;
    }
    return item;
};

// Creates then appends a collection into the parent children with the specified
// underlying protobuf message.
inline ModelItem *appendRowCollection(pb::mission::Mission::Collection *collection, ModelItem *parent)
{
    auto item = appendRowItem(collection, parent);
    for (auto &element : *collection->mutable_elements()) {
        appendRowElement(&element, item);
    }
    return item;
};

// Creates then appends a mission into the parent children with the specified
// underlying protobuf message.
inline ModelItem *appendRowMission(pb::mission::Mission *mission, ModelItem *parent)
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

ModelItem::ModelItem(google::protobuf::Message *protobuf, ModelItem *parent)
    : _parent(parent)
    , _backend(protobuf, this)
{
}

ModelItem::~ModelItem()
{
    qDeleteAll(_childs);
}

// Returns the child specified by the given row.
ModelItem *ModelItem::child(int row)
{
    Q_ASSERT_X(row >= 0 && row < _childs.size(), CLASSNAME, "fail getting child");

    return _childs.at(row);
}

// Returns the row of the item into the parent children.
int ModelItem::row() const
{
    return _parent ? _parent->_childs.indexOf(const_cast<ModelItem *>(this)) : 0;
}

// Removes the child item specified by the given row. This also removes the
// underlying protobuf data through the backend.
void ModelItem::removeRow(int row)
{
    Q_ASSERT_X(row >= 0 && row < _childs.size(), CLASSNAME, "fail removing row");

    _backend.removeRow(row);
    auto *pointer = child(row);
    _childs.remove(row);
    delete pointer;
    pointer = nullptr;
}

// TODO
//void ModelItem::insertRow(int row)
//{
//    Q_ASSERT_X(row >= 0 && row < _childs.size(), CLASSNAME, "fail inserting row");

//    //    auto new_protobuf = _backend.appendRow(ModelBacken::kPoint);
//    //    new_protobuf->CopyFrom(protobuf);
//    //    appendRow(new_protobuf);
//    //    _backend.moveUpLastRowAt(row);
//    //    _childs.insert(row, _childs.last());
//}

//// TODO
//void ModelItem::insertRow(const int row, const google::protobuf::Message &protobuf)
//{
//    Q_ASSERT_X(row >= 0 && row < _childs.size(), CLASSNAME, "fail inserting row");

//    auto new_protobuf = _backend.appendRow(ModelBacken::component(&protobuf));
//    new_protobuf->CopyFrom(protobuf);
//    appendRow(new_protobuf);
//    _backend.moveUpLastRowAt(row);
//    _childs.insert(row, _childs.last());
//}

// Appends a child into the parent children with the specified underlying protobuf
// message.
void ModelItem::appendRow(google::protobuf::Message *protobuf)
{
    if (!protobuf) {
        qWarning() << CLASSNAME << "[Warning] fail appending row, null protobuf pointer";
        return;
    }

    const auto &component_id = ModelBacken::component(protobuf);

    if (component_id == ModelBacken::kMission) {
        appendRowMission(static_cast<pb::mission::Mission *>(protobuf), this);

    } else if (component_id == ModelBacken::kCollection) {
        appendRowCollection(static_cast<pb::mission::Mission::Collection *>(protobuf), this);

    } else if (component_id == ModelBacken::kElement) {
        appendRowElement(static_cast<pb::mission::Mission::Element *>(protobuf), this);

    } else if (component_id == ModelBacken::kPoint) {
        appendRowPoint(static_cast<pb::mission::Mission::Element::Point *>(protobuf), this);

    } else if (component_id == ModelBacken::kRail) {
        appendRowRail(static_cast<pb::mission::Mission::Element::Rail *>(protobuf), this);

    } else if (component_id == ModelBacken::kSegment) {
        appendRowSegment(static_cast<pb::mission::Mission::Element::Segment *>(protobuf), this);

    } else {
        qWarning() << CLASSNAME << "[Warning] fail appending row, missing definition" << component_id;
    }
}

// Appends a child into the parent children with the specified action.
void ModelItem::appendRow(const ModelBacken::Component component)
{
    appendRow(_backend.appendRow(component));
}

//// Returns the data specified by the column.
// QVariant ModelItem::data(int column) const
//{
//    if (column < 0 || column >= COLUMN_COUT) return QVariant();

//    return _backend.data(column);
//}
//// Sets the data at the specified coloum and value. Returns true if successful
//// otherwise returns false.
// bool ModelItem::setData(int column, const QVariant &value)
//{
//    if (column < 0 || column >= COLUMN_COUT) return false;

//    return _backend.setData(column, value);
//}
