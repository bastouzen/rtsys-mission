// ===
// === Include
// ============================================================================ //

#include "mission/manager.h"
#include "widget/tree.h"

#include <QApplication>
#include <QLoggingCategory>

// ===
// === Main
// ============================================================================ //

int main(int argc, char *argv[])
{
    QLoggingCategory::setFilterRules("rtsys.*.debug=true");

    QApplication app(argc, argv);
    MissionManager manager;

    MissionTreeWidget widget;
    widget.setWindowTitle(QObject::tr("RTSys Mission Tree Widget"));
    widget.setManager(&manager);
    widget.show();

    // manager.loadMission(MissionManager::getMissionTemplate());

    return app.exec();
}

/* IMPLEMENTED FEATURES
 * - reverse children order (permutate) for route, family, rail and segment)
 * - drag & drop (single and multiple selection)
 * - remove (single and multiple selection)
 */

/*
 * TODO
 *  Edit (generate dialogue windows depending on the flag identifier)
 *  Add contact
 *      Allow cast point->contact and contact ->point
 *  Add Device
 *      Allow cast collection->device and device ->collection
 * Add Copy/Past
 */
