// ===
// === Include
// ============================================================================ //

#include "manager.h"
#include "widget/tree.h"

#include "protobuf/mission.pb.h"

#include <QApplication>
#include <QDebug>

// ===
// === Main
// ============================================================================ //

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Setup some data
    pb::mission::Mission mission;
    mission.set_name("RTSys Mission Template");

    pb::mission::Mission::Component *component;
    pb::mission::Mission::Element *element;

    component = mission.add_components();
    component->mutable_element()->mutable_point()->set_name("P0");

    component = mission.add_components();
    component->mutable_collection()->set_name("Scenario");

    element = component->mutable_collection()->add_elements();
    element->mutable_point()->set_name("P1");

    element = component->mutable_collection()->add_elements();
    element->mutable_rail()->set_name("R0");
    element->mutable_rail()->mutable_p0()->set_name("RA");
    element->mutable_rail()->mutable_p1()->set_name("RB");

    element = component->mutable_collection()->add_elements();
    element->mutable_segment()->set_name("S1");
    element->mutable_segment()->mutable_p0()->set_name("SA");
    element->mutable_segment()->mutable_p1()->set_name("SB");

    component = mission.add_components();
    component->mutable_element()->mutable_rail()->set_name("R0");
    component->mutable_element()->mutable_rail()->mutable_p0()->set_name("RA");
    component->mutable_element()->mutable_rail()->mutable_p1()->set_name("RB");

    component = mission.add_components();
    component->mutable_collection()->set_name("Route");
    element = component->mutable_collection()->add_elements();
    element->mutable_point()->set_name("R0");
    element = component->mutable_collection()->add_elements();
    element->mutable_point()->set_name("R1");
    element = component->mutable_collection()->add_elements();
    element->mutable_point()->set_name("R2");
    element = component->mutable_collection()->add_elements();
    element->mutable_point()->set_name("R3");
    element = component->mutable_collection()->add_elements();
    element->mutable_point()->set_name("R4");

    component = mission.add_components();
    component->mutable_element()->mutable_segment()->set_name("S0");
    component->mutable_element()->mutable_segment()->mutable_p0()->set_name("SA");
    component->mutable_element()->mutable_segment()->mutable_p1()->set_name("SB");

    component = mission.add_components();
    component->mutable_collection()->set_name("Family");
    element = component->mutable_collection()->add_elements();
    element->mutable_rail()->set_name("J1");
    element->mutable_rail()->mutable_p0()->set_name("J1A");
    element->mutable_rail()->mutable_p1()->set_name("J1B");
    element = component->mutable_collection()->add_elements();
    element->mutable_rail()->set_name("J2");
    element->mutable_rail()->mutable_p0()->set_name("J2A");
    element->mutable_rail()->mutable_p1()->set_name("J2B");
    element = component->mutable_collection()->add_elements();
    element->mutable_rail()->set_name("J3");
    element->mutable_rail()->mutable_p0()->set_name("J3A");
    element->mutable_rail()->mutable_p1()->set_name("J3B");
    element = component->mutable_collection()->add_elements();
    element->mutable_rail()->set_name("J4");
    element->mutable_rail()->mutable_p0()->set_name("J4A");
    element->mutable_rail()->mutable_p1()->set_name("J4B");
    element = component->mutable_collection()->add_elements();
    element->mutable_rail()->set_name("J5");
    element->mutable_rail()->mutable_p0()->set_name("J5A");
    element->mutable_rail()->mutable_p1()->set_name("J5B");

    MissionManager manager;

    MissionTreeWidget widget;
    widget.setWindowTitle(QObject::tr("RTSys Mission Tree Widget"));
    widget.setManager(&manager);
    widget.show();

    manager.loadMission(mission);

    return app.exec();
}
