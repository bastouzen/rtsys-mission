#ifndef RTSYS_MISSION_MODEL_MISC_H
#define RTSYS_MISSION_MODEL_MISC_H

// ===
// === Include
// ============================================================================ //

#include "private/model.h"
#include "protobuf/mission.pb.h"

// ===
// === Function
// ============================================================================ //

namespace misc {

template <class T>
inline MissionItem *appendItem(T *protobuf, MissionItem *parent)
{
    assert(protobuf);
    auto *child = new MissionItem(
        {QString::fromStdString(protobuf->GetDescriptor()->name()), QString::fromStdString(protobuf->name())}, protobuf,
        parent);
    parent->appendChild(child);
    return child;
}

template <class T>
inline MissionItem *appendRail(T *protobuf, MissionItem *parent)
{
    assert(protobuf);
    auto *elder = appendItem(protobuf, parent);
    appendItem(protobuf->mutable_p0(), elder);
    appendItem(protobuf->mutable_p1(), elder);
}

inline void appendElement(pb::mission::Mission::Element *element, MissionItem *parent)
{
    MissionItem *elder;
    switch (element->element_case()) {
        case pb::mission::Mission::Element::kPoint:
            appendItem(element->mutable_point(), parent);
            break;
        case pb::mission::Mission::Element::kRail:
            elder = appendItem(element->mutable_rail(), parent);
            appendItem(element->mutable_rail()->mutable_p0(), elder);
            appendItem(element->mutable_rail()->mutable_p1(), elder);
            break;
        case pb::mission::Mission::Element::kSegment:
            elder = appendItem(element->mutable_segment(), parent);
            appendItem(element->mutable_segment()->mutable_p0(), elder);
            appendItem(element->mutable_segment()->mutable_p1(), elder);
            break;
        default:
            break;
    }
};

inline void appendCollection(pb::mission::Mission::Collection *collection, MissionItem *parent)
{
    auto *elder = appendItem(collection, parent);
    for (auto &element : *collection->mutable_elements()) {
        appendElement(&element, elder);
    }
};

} // namespace misc

#endif // RTSYS_MISSION_MODEL_MISC_H
