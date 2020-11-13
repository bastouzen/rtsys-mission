DISTFILES += \
    $$PWD/mission.proto

HEADERS += \
    $$PWD/mission.pb.h \
    $$PWD/misc/misc_cpp.h \
    $$PWD/misc/misc_cpp_delimited.h

SOURCES += \
    $$PWD/mission.pb.cc \
    $$PWD/misc/misc_cpp_delimited.cpp


LIBS += \
    -lprotobuf
