QT += widgets
requires(qtConfig(treeview))

include(protobuf/protobuf.pri)
include(mission/mission.pri)

# Widget
FORMS += widget/tree.ui
HEADERS += widget/tree.h
SOURCES += widget/tree.cpp

# Main
SOURCES += main.cpp
RESOURCES += resource.qrc
