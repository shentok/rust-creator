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
    tools/nimlexer.h \
    tools/sourcecodestream.h \
    project/nimbuildsystem.h \
    project/nimproject.h \
    project/nimprojectnode.h \
    project/nimbuildconfiguration.h \
    project/nimcompilerbuildstep.h \
    project/nimcompilerbuildstepconfigwidget.h \
    project/nimcompilercleanstep.h \
    project/nimrunconfiguration.h \
    settings/nimcodestylesettingspage.h \
    settings/nimcodestylepreferencesfactory.h \
    settings/nimsettings.h \
    settings/nimcodestylepreferenceswidget.h \
    settings/nimtoolssettingspage.h \
    project/nimtoolchain.h \
    project/nimtoolchainfactory.h \
    suggest/client.h \
    suggest/clientrequests.h \
    suggest/nimsuggest.h \
    suggest/nimsuggestcache.h \
    suggest/server.h \
    suggest/sexprlexer.h \
    suggest/sexprparser.h

SOURCES += \
    nimplugin.cpp \
    editor/nimindenter.cpp \
    tools/nimlexer.cpp \
    project/nimbuildsystem.cpp \
    project/nimproject.cpp \
    project/nimprojectnode.cpp \
    project/nimbuildconfiguration.cpp \
    project/nimcompilerbuildstep.cpp \
    project/nimcompilerbuildstepconfigwidget.cpp \
    project/nimcompilercleanstep.cpp \
    project/nimrunconfiguration.cpp \
    settings/nimcodestylesettingspage.cpp \
    settings/nimcodestylepreferencesfactory.cpp \
    settings/nimsettings.cpp \
    settings/nimcodestylepreferenceswidget.cpp \
    settings/nimtoolssettingspage.cpp \
    project/nimtoolchain.cpp \
    project/nimtoolchainfactory.cpp \
    suggest/client.cpp \
    suggest/clientrequests.cpp \
    suggest/nimsuggest.cpp \
    suggest/nimsuggestcache.cpp \
    suggest/server.cpp

FORMS += \
    project/nimcompilerbuildstepconfigwidget.ui \
    settings/nimcodestylepreferenceswidget.ui \
    settings/nimtoolssettingswidget.ui

include(nim_dependencies.pri)

include($$QTCREATOR_SOURCES/src/qtcreatorplugin.pri)
