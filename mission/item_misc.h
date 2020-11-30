/*
 * This defines some miscellaneous functions used by MissionItem.
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

// Insert a device protobuf message depending on the parent component to
// the specified protobuf message and row.
inline MissionItem::Protobuf *insertDeviceProtobuf(const int row, MissionItem *parent)
{
    auto *protobuf = parent->data(Qt::UserRoleWrapper).value<MissionItem::Wrapper>().pointer;

    const auto &feature = MissionItem::Features(parent->data(Qt::UserRoleFeature).toInt());
    if (feature & MissionItem::kMission) {
        auto *mission = static_cast<rtsys::mission::Mission *>(protobuf);
        auto *component = mission->add_components();
        auto *device = component->mutable_device();
        if (mission->mutable_components(row) != component) {
            RepeatedFieldMoveUpLastAt(row, mission->mutable_components());
        }
        return device;
    }
    return nullptr;
};

// Insert a collection protobuf message depending on the parent component to
// the specified protobuf message and row.
inline MissionItem::Protobuf *insertCollectionProtobuf(const int row, MissionItem *parent)
{
    auto *protobuf = parent->data(Qt::UserRoleWrapper).value<MissionItem::Wrapper>().pointer;

    const auto &feature = MissionItem::Features(parent->data(Qt::UserRoleFeature).toInt());
    if (feature & MissionItem::kMission) {
        auto *mission = static_cast<rtsys::mission::Mission *>(protobuf);
        auto *component = mission->add_components();
        auto *collection = component->mutable_collection();
        if (mission->mutable_components(row) != component) {
            RepeatedFieldMoveUpLastAt(row, mission->mutable_components());
        }
        return collection;

    } else if (feature & MissionItem::kDevice) {
        auto *device = static_cast<rtsys::mission::Device *>(protobuf);
        auto *component = device->add_components();
        auto *collection = component->mutable_collection();
        if (device->mutable_components(row) != component) {
            RepeatedFieldMoveUpLastAt(row, device->mutable_components());
        }
        return collection;
    }
    return nullptr;
};

// Insert a rail protobuf message depending on the parent component to the
// specified protobuf message and row.
inline MissionItem::Protobuf *insertLineProtobuf(const int row, MissionItem *parent)
{
    auto *protobuf = parent->data(Qt::UserRoleWrapper).value<MissionItem::Wrapper>().pointer;

    const auto &feature = MissionItem::Features(parent->data(Qt::UserRoleFeature).toInt());
    if (feature & MissionItem::kMission) {
        auto *mission = static_cast<rtsys::mission::Mission *>(protobuf);
        auto *component = mission->add_components();
        auto *line = component->mutable_block()->mutable_line();
        if (mission->mutable_components(row) != component) {
            RepeatedFieldMoveUpLastAt(row, mission->mutable_components());
        }
        return line;

    } else if (feature & MissionItem::kDevice) {
        auto *device = static_cast<rtsys::mission::Device *>(protobuf);
        auto *component = device->add_components();
        auto *line = component->mutable_block()->mutable_line();
        if (device->mutable_components(row) != component) {
            RepeatedFieldMoveUpLastAt(row, device->mutable_components());
        }
        return line;

    } else if (feature & MissionItem::kCollection) {
        auto *component = static_cast<rtsys::mission::Collection *>(protobuf);
        auto *block = component->add_blocks();
        auto *line = block->mutable_line();
        if (component->mutable_blocks(row) != block) {
            RepeatedFieldMoveUpLastAt(row, component->mutable_blocks());
        }
        return line;
    }

    return nullptr;
};

// Insert a point protobuf message depending on the parent component to the
// specified protobuf message and row.
inline MissionItem::Protobuf *insertPointProtobuf(const int row, MissionItem *parent)
{
    auto *protobuf = parent->data(Qt::UserRoleWrapper).value<MissionItem::Wrapper>().pointer;

    const auto &feature = MissionItem::Features(parent->data(Qt::UserRoleFeature).toInt());
    if (feature & MissionItem::kMission) {
        auto *mission = static_cast<rtsys::mission::Mission *>(protobuf);
        auto *component = mission->add_components();
        auto *point = component->mutable_block()->mutable_point();
        if (mission->mutable_components(row) != component) {
            RepeatedFieldMoveUpLastAt(row, mission->mutable_components());
        }
        return point;

    } else if (feature & MissionItem::kDevice) {
        auto *device = static_cast<rtsys::mission::Device *>(protobuf);
        auto *component = device->add_components();
        auto *point = component->mutable_block()->mutable_point();
        if (device->mutable_components(row) != component) {
            RepeatedFieldMoveUpLastAt(row, device->mutable_components());
        }
        return point;

    } else if (feature & MissionItem::kCollection) {
        auto *component = static_cast<rtsys::mission::Collection *>(protobuf);
        auto *block = component->add_blocks();
        auto *point = block->mutable_point();
        if (component->mutable_blocks(row) != block) {
            RepeatedFieldMoveUpLastAt(row, component->mutable_blocks());
        }
        return point;
    }

    else if (feature & MissionItem::kLine) {
        auto *line = static_cast<rtsys::mission::Block::Line *>(protobuf);
        auto *point = line->add_points();
        if (line->mutable_points(row) != point) {
            RepeatedFieldMoveUpLastAt(row, line->mutable_points());
        }
        return point;
    }

    return nullptr;
};

// Adds a child item into the parent children and then set its data from the specified
// protobuf message.
inline MissionItem *addChild(MissionItem::Protobuf *protobuf, MissionItem *parent)
{
    parent->insertChild();                                // add child at last position
    auto *item = parent->child(parent->countChild() - 1); // retrieve the created item and set it
    item->setData(QVariant::fromValue(MissionItem::wrap(protobuf)), Qt::UserRoleWrapper);
    return item;
}

// Expands the point protobuf message from its parent item.
inline void expandPointProtobuf(rtsys::mission::Block::Point *point, MissionItem *parent, bool is_nested = false)
{
    if (is_nested) addChild(point, parent);
};

// Expands the line protobuf message from its parent item.
inline void expandLineProtobuf(rtsys::mission::Block::Line *line, MissionItem *parent, bool is_nested = false)
{
    auto item = is_nested ? addChild(line, parent) : parent;
    addChild(line->mutable_points(0), item);
    addChild(line->mutable_points(1), item);
};

// Expands the block protobuf message from its parent item.
inline void expandBlockProtobuf(rtsys::mission::Block *block, MissionItem *parent, bool is_nested = false)
{
    switch (block->block_case()) {
        case rtsys::mission::Block::kPoint:
            expandPointProtobuf(block->mutable_point(), parent, is_nested);
            break;
        case rtsys::mission::Block::kLine:
            expandLineProtobuf(block->mutable_line(), parent, is_nested);
            break;
        default:
            break;
    }
};

// Expands the collection protobuf message from its parent item.
inline void expandCollectionProtobuf(rtsys::mission::Collection *collection, MissionItem *parent,
                                     bool is_nested = false)
{
    auto item = is_nested ? addChild(collection, parent) : parent;
    for (auto &block : *collection->mutable_blocks()) {
        expandBlockProtobuf(&block, item, true); // force nested flag
    }
};

// Expands the device protobuf message from its parent item.
inline void expandDeviceProtobuf(rtsys::mission::Device *device, MissionItem *parent, bool is_nested = false)
{
    auto item = is_nested ? addChild(device, parent) : parent;
    for (auto &component : *device->mutable_components()) {
        switch (component.component_case()) {
            case rtsys::mission::Device::Component::kCollection:
                expandCollectionProtobuf(component.mutable_collection(), item, true); // nested
                break;
            case rtsys::mission::Device::Component::kBlock:
                expandBlockProtobuf(component.mutable_block(), item, true); // nested
                break;
            default:
                break;
        }
    }
};

// Expands the mission protobuf message from its parent item.
inline void expandMissionProtobuf(rtsys::mission::Mission *mission, MissionItem *parent)
{
    for (auto &component : *mission->mutable_components()) {
        switch (component.component_case()) {
            case rtsys::mission::Mission::Component::kDevice:
                expandDeviceProtobuf(component.mutable_device(), parent, true); // nested
                break;
            case rtsys::mission::Mission::Component::kCollection:
                expandCollectionProtobuf(component.mutable_collection(), parent, true); // nested
                break;
            case rtsys::mission::Mission::Component::kBlock:
                expandBlockProtobuf(component.mutable_block(), parent, true); // nested
                break;
            default:
                break;
        }
    }
};

#endif // RTSYS_MISSION_MODEL_ITEM_H
