QT += widgets
requires(qtConfig(treeview))

# Protbuf
OTHER_FILES += protobuf/mission.proto
HEADERS += protobuf/mission.pb.h
SOURCES += protobuf/mission.pb.cc
LIBS += -lprotobuf

# Model
HEADERS += private/model.h private/backend.h
SOURCES += private/model.cpp private/backend.cpp


# Widget
FORMS += widget/tree.ui
HEADERS += widget/tree.h
SOURCES += widget/tree.cpp

# Main
HEADERS += manager.h
SOURCES += manager.cpp main.cpp
RESOURCES += resource.qrc
