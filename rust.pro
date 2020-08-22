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
    project/nimbuildsystem.h \
    project/nimproject.h \
    project/nimprojectnode.h \
    project/nimbuildconfiguration.h \
    project/nimbuildconfigurationwidget.h \
    project/nimcompilerbuildstep.h \
    project/nimcompilerbuildstepconfigwidget.h \
    project/nimrunconfiguration.h \
    project/nimtoolchain.h \
    project/nimtoolchainfactory.h \

SOURCES += \
    nimplugin.cpp \
    project/nimbuildsystem.cpp \
    project/nimproject.cpp \
    project/nimprojectnode.cpp \
    project/nimbuildconfiguration.cpp \
    project/nimbuildconfigurationwidget.cpp \
    project/nimcompilerbuildstep.cpp \
    project/nimcompilerbuildstepconfigwidget.cpp \
    project/nimrunconfiguration.cpp \
    project/nimtoolchain.cpp \
    project/nimtoolchainfactory.cpp \

FORMS += \
    project/nimbuildconfigurationwidget.ui \
    project/nimcompilerbuildstepconfigwidget.ui \

include(nim_dependencies.pri)

include($$QTCREATOR_SOURCES/src/qtcreatorplugin.pri)
