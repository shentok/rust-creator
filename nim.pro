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
    editor/nimindenter.h \
    project/nimblebuildconfiguration.h \
    project/nimblebuildstep.h \
    project/nimbleproject.h \
    project/nimblerunconfiguration.h \
    project/nimbletaskstep.h \
    project/nimoutputtaskparser.h \
    tools/nimlexer.h \
    tools/sourcecodestream.h \
    project/nimbuildsystem.h \
    project/nimblebuildsystem.h \
    project/nimproject.h \
    project/nimbuildconfiguration.h \
    project/nimcompilerbuildstep.h \
    project/nimcompilercleanstep.h \
    project/nimrunconfiguration.h \
    settings/nimcodestylesettingspage.h \
    settings/nimcodestylepreferencesfactory.h \
    settings/nimsettings.h \
    settings/nimcodestylepreferenceswidget.h \
    project/nimtoolchain.h \
    project/nimtoolchainfactory.h \

SOURCES += \
    nimplugin.cpp \
    editor/nimindenter.cpp \
    project/nimblebuildconfiguration.cpp \
    project/nimblebuildstep.cpp \
    project/nimbletaskstep.cpp \
    project/nimbleproject.cpp \
    project/nimblerunconfiguration.cpp \
    project/nimoutputtaskparser.cpp \
    tools/nimlexer.cpp \
    project/nimbuildsystem.cpp \
    project/nimblebuildsystem.cpp \
    project/nimproject.cpp \
    project/nimbuildconfiguration.cpp \
    project/nimcompilerbuildstep.cpp \
    project/nimcompilercleanstep.cpp \
    project/nimrunconfiguration.cpp \
    settings/nimcodestylesettingspage.cpp \
    settings/nimcodestylepreferencesfactory.cpp \
    settings/nimsettings.cpp \
    settings/nimcodestylepreferenceswidget.cpp \
    project/nimtoolchain.cpp \
    project/nimtoolchainfactory.cpp \

FORMS += \
    settings/nimcodestylepreferenceswidget.ui

include(nim_dependencies.pri)

include($$QTCREATOR_SOURCES/src/qtcreatorplugin.pri)
