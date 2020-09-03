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
    project/nimblebuildstepwidget.h \
    project/nimbleproject.h \
    project/nimblerunconfiguration.h \
    project/nimbletaskstep.h \
    project/nimbletaskstepwidget.h \
    project/nimbuildsystem.h \
    project/nimblebuildsystem.h \
    project/nimprojectnode.h \
    project/nimtoolchain.h \
    project/nimtoolchainfactory.h \

SOURCES += \
    nimplugin.cpp \
    project/nimblebuildconfiguration.cpp \
    project/nimblebuildstep.cpp \
    project/nimbletaskstep.cpp \
    project/nimblebuildstepwidget.cpp \
    project/nimbleproject.cpp \
    project/nimblerunconfiguration.cpp \
    project/nimbletaskstepwidget.cpp \
    project/nimbuildsystem.cpp \
    project/nimblebuildsystem.cpp \
    project/nimprojectnode.cpp \
    project/nimtoolchain.cpp \
    project/nimtoolchainfactory.cpp \

FORMS += \
    project/nimblebuildstepwidget.ui \
    project/nimbletaskstepwidget.ui \

include(nim_dependencies.pri)

include($$QTCREATOR_SOURCES/src/qtcreatorplugin.pri)
