QTCREATOR_SOURCES = $$QTC_SOURCE
IDE_BUILD_TREE = $$QTC_BUILD

DEFINES += \
    NIM_LIBRARY

RESOURCES += \
    nim.qrc

INCLUDEPATH += $$PWD

HEADERS += \
    nimplugin.h \
    nimconstants.h \
    project/nimblebuildconfiguration.h \
    project/nimblebuildstep.h \
    project/nimbleproject.h \
    project/nimblerunconfiguration.h \
    project/nimbletaskstep.h \
    project/nimoutputtaskparser.h \
    project/nimbuildsystem.h \
    project/nimblebuildsystem.h \
    project/nimtoolchain.h \
    project/nimtoolchainfactory.h \

SOURCES += \
    nimplugin.cpp \
    project/nimblebuildconfiguration.cpp \
    project/nimblebuildstep.cpp \
    project/nimbletaskstep.cpp \
    project/nimbleproject.cpp \
    project/nimblerunconfiguration.cpp \
    project/nimoutputtaskparser.cpp \
    project/nimbuildsystem.cpp \
    project/nimblebuildsystem.cpp \
    project/nimtoolchain.cpp \
    project/nimtoolchainfactory.cpp \

include(nim_dependencies.pri)

include($$QTCREATOR_SOURCES/src/qtcreatorplugin.pri)
