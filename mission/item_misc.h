/*
 * This defines the model item object. It represents one item of the model tree.
 * Each one holds a reference to their parent and to their children. The parent
 * reference is weak so that it isn't responsible for deleting its parent. The
 * children reference is strong so that it is responsible for deleting them. It
 * holds the data that are displayed in the model tree view.
 *
 * The backend is responsible of managing the underlying protobuf message.
 */

#ifndef RTSYS_MISSION_MODEL_ITEM_MISC_H
#define RTSYS_MISSION_MODEL_ITEM_MISC_H

// ===
// === Include
// ============================================================================ //

#include <google/protobuf/repeated_field.h>

#include "mission/item.h"
#include "protobuf/mission.pb.h"

// ===
// === Function
// ============================================================================ //

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
    for (int i = repeated->size() - 1; i > row; i--) {
        repeated->SwapElements(i, i - 1);
    }
}

// Insert a collection protobuf message depending on the parent flag identifier
// to the specified protobuf message and row.
inline MissionItem::Protobuf *insertCollectionProtobuf(const int row, MissionItem *parent)
{
    auto *protobuf = parent->data(Qt::UserRoleWrapper).value<MissionItem::Wrapper>().pointer;

    const auto &parent_flag_id = parent->data(Qt::UserRoleFlag);
    if (parent_flag_id == MissionItem::kMission) {
        auto *mission = static_cast<pb::mission::Mission *>(protobuf);
        auto *component = mission->add_components();
        auto *collection = component->mutable_collection();
        if (mission->mutable_components(row) != component) {
            RepeatedFieldMoveUpLastAt(row, mission->mutable_components());
        }
        return collection;
    }
    return nullptr;
};

// Insert a point protobuf message depending on the parent flag identifier
// to the specified protobuf message and row.
inline MissionItem::Protobuf *insertPointProtobuf(const int row, MissionItem *parent)
{
    auto *protobuf = parent->data(Qt::UserRoleWrapper).value<MissionItem::Wrapper>().pointer;

    const auto &parent_flag_id = parent->data(Qt::UserRoleFlag);
    if (parent_flag_id == MissionItem::kMission) {
        auto *mission = static_cast<pb::mission::Mission *>(protobuf);
        auto *component = mission->add_components();
        auto *point = component->mutable_element()->mutable_point();
        if (mission->mutable_components(row) != component) {
            RepeatedFieldMoveUpLastAt(row, mission->mutable_components());
        }
        return point;

    } else if (parent_flag_id == MissionItem::kCollection) {
        auto *component = static_cast<pb::mission::Mission::Collection *>(protobuf);
        auto *element = component->add_elements();
        auto *point = element->mutable_point();
        if (component->mutable_elements(row) != element) {
            RepeatedFieldMoveUpLastAt(row, component->mutable_elements());
        }
        return point;
    }

    return nullptr;
};

// Insert a rail protobuf message depending on the parent flag identifier
// to the specified protobuf message and row.
inline MissionItem::Protobuf *insertRailProtobuf(const int row, MissionItem *parent)
{
    auto *protobuf = parent->data(Qt::UserRoleWrapper).value<MissionItem::Wrapper>().pointer;

    const auto &parent_flag_id = parent->data(Qt::UserRoleFlag);
    if (parent_flag_id == MissionItem::kMission) {
        auto *mission = static_cast<pb::mission::Mission *>(protobuf);
        auto *component = mission->add_components();
        auto *rail = component->mutable_element()->mutable_rail();
        if (mission->mutable_components(row) != component) {
            RepeatedFieldMoveUpLastAt(row, mission->mutable_components());
        }
        return rail;

    } else if (parent_flag_id == MissionItem::kCollection) {
        auto *component = static_cast<pb::mission::Mission::Collection *>(protobuf);
        auto *element = component->add_elements();
        auto *rail = element->mutable_rail();
        if (component->mutable_elements(row) != element) {
            RepeatedFieldMoveUpLastAt(row, component->mutable_elements());
        }
        return rail;
    }

    return nullptr;
};

// Insert a segment protobuf message depending on the parent flag identifier
// to the specified protobuf message and row.
inline MissionItem::Protobuf *insertSegmentProtobuf(const int row, MissionItem *parent)
{
    auto *protobuf = parent->data(Qt::UserRoleWrapper).value<MissionItem::Wrapper>().pointer;

    const auto &parent_flag_id = parent->data(Qt::UserRoleFlag);
    if (parent_flag_id == MissionItem::kMission) {
        auto *mission = static_cast<pb::mission::Mission *>(protobuf);
        auto *component = mission->add_components();
        auto *segment = component->mutable_element()->mutable_segment();
        if (mission->mutable_components(row) != component) {
            RepeatedFieldMoveUpLastAt(row, mission->mutable_components());
        }
        return segment;

    } else if (parent_flag_id == MissionItem::kCollection) {
        auto *component = static_cast<pb::mission::Mission::Collection *>(protobuf);
        auto *element = component->add_elements();
        auto *segment = element->mutable_segment();
        if (component->mutable_elements(row) != element) {
            RepeatedFieldMoveUpLastAt(row, component->mutable_elements());
        }
        return segment;
    }

    return nullptr;
};

// Adds a child item into the parent children and then set its data from the specified
// protobuf message.
inline MissionItem *addChild(MissionItem::Protobuf *protobuf, MissionItem *parent, bool insert_child = true)
{
    if (insert_child) parent->insertChild();              // add child at last position
    auto *item = parent->child(parent->countChild() - 1); // retrieve the created item and set it
    item->setData(QVariant::fromValue(MissionItem::wrap(protobuf)), Qt::UserRoleWrapper);
    return item;
}

// Expands the point protobuf message from its parent item.
inline void expandPointProtobuf(pb::mission::Mission::Element::Point *point, MissionItem *parent,
                                bool insert_child = true)
{
    addChild(point, parent, insert_child);
};

// Expands the rail protobuf message from its parent item.
inline void expandRailProtobuf(pb::mission::Mission::Element::Rail *rail, MissionItem *parent, bool insert_child = true)
{
    auto item = addChild(rail, parent, insert_child);
    addChild(rail->mutable_p0(), item);
    addChild(rail->mutable_p1(), item);
};

// Expands the segment protobuf message from its parent item.
inline void expandSegmentProtobuf(pb::mission::Mission::Element::Segment *segment, MissionItem *parent,
                                  bool insert_child = true)
{
    auto item = addChild(segment, parent, insert_child);
    addChild(segment->mutable_p0(), item);
    addChild(segment->mutable_p1(), item);
};

// Expands the element protobuf message from its parent item.
inline void expandElementProtobuf(pb::mission::Mission::Element *element, MissionItem *parent, bool insert_child = true)
{
    switch (element->element_case()) {
        case pb::mission::Mission::Element::kPoint:
            expandPointProtobuf(element->mutable_point(), parent, insert_child);
            break;
        case pb::mission::Mission::Element::kRail:
            expandRailProtobuf(element->mutable_rail(), parent, insert_child);
            break;
        case pb::mission::Mission::Element::kSegment:
            expandSegmentProtobuf(element->mutable_segment(), parent, insert_child);
            break;
        default:
            break;
    }
};

// Expands the collection protobuf message from its parent item.
inline void expandCollectionProtobuf(pb::mission::Mission::Collection *collection, MissionItem *parent,
                                     bool make_parent = true)
{
    auto item = make_parent ? addChild(collection, parent) : parent;
    for (auto &element : *collection->mutable_elements()) {
        expandElementProtobuf(&element, item);
    }
};

// Expands the mission protobuf message from its parent item.
inline void expandMissionProtobuf(pb::mission::Mission *mission, MissionItem *parent)
{
    for (auto &component : *mission->mutable_components()) {
        switch (component.component_case()) {
            case pb::mission::Mission::Component::kElement:
                expandElementProtobuf(component.mutable_element(), parent);
                break;
            case pb::mission::Mission::Component::kCollection:
                expandCollectionProtobuf(component.mutable_collection(), parent);
                break;
            default:
                break;
        }
    }
};

#endif // RTSYS_MISSION_MODEL_ITEM_H
