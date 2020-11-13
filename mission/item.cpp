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

// Creates then appends an element into the parent children with the specified
// underlying protobuf message.
inline ModelItem *appendRowElement(pb::mission::Mission::Element *element, ModelItem *parent)
{
    ModelItem *item = nullptr;
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
    if (row < 0 || row >= _childs.size()) return nullptr;

    return _childs.at(row);
}

// Returns the data specified by the column.
QVariant ModelItem::data(int column) const
{
    if (column < 0 || column >= COLUMN_COUT) return QVariant();

    return _backend.data(column);
}

// Returns the row of the item into the parent children.
int ModelItem::row() const
{
    if (_parent) {
        return _parent->_childs.indexOf(const_cast<ModelItem *>(this));
    }
    return 0;
}

// Sets the data at the specified coloum and value. Returns true if successful
// otherwise returns false.
bool ModelItem::setData(int column, const QVariant &value)
{
    if (column < 0 || column >= COLUMN_COUT) return false;

    return _backend.setData(column, value);
}

// Creates then appends a child into the parent children with the specified
// underlying protobuf message.
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
    } else {
        appendRowElement(static_cast<pb::mission::Mission::Element *>(protobuf), this);
    }
}

// Creates then appends a child into the parent children with the specified action.
void ModelItem::appendRow(const ModelBacken::Action action)
{
    appendRow(_backend.appendRow(action));
}

// Removes the child specified by the given row. This also removes the underlying
// protobuf data through the backend.
void ModelItem::removeRow(int row)
{
    if (row < 0 || row >= _childs.size()) {
        qWarning() << CLASSNAME << "[Warning] fail removing row, row overrange";
        return;
    }

    _backend.removeRow(row);
    auto *pointer = child(row);
    _childs.remove(row);
    delete pointer;
    pointer = nullptr;
}
