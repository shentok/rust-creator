cmake_minimum_required(VERSION 3.10)

project(Nim)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 17)

find_package(QtCreator COMPONENTS Core TextEditor ProjectExplorer REQUIRED)
find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
set(QtX Qt${QT_VERSION_MAJOR})

add_qtc_plugin(Nim
  PLUGIN_DEPENDS Core TextEditor ProjectExplorer
  SOURCES
    editor/nimcompletionassistprovider.cpp editor/nimcompletionassistprovider.h
    editor/nimeditorfactory.cpp editor/nimeditorfactory.h
    editor/nimhighlighter.cpp editor/nimhighlighter.h
    editor/nimindenter.cpp editor/nimindenter.h
    editor/nimtexteditorwidget.cpp editor/nimtexteditorwidget.h
    nim.qrc
    nimconstants.h
    nimplugin.cpp nimplugin.h
    project/nimblebuildstep.h project/nimblebuildstep.cpp
    project/nimbleproject.h project/nimbleproject.cpp
    project/nimblerunconfiguration.h project/nimblerunconfiguration.cpp
    project/nimbletaskstep.h project/nimbletaskstep.cpp
    project/nimblebuildsystem.h project/nimblebuildsystem.cpp
    project/nimblebuildconfiguration.h project/nimblebuildconfiguration.cpp
    project/nimbuildsystem.cpp project/nimbuildsystem.h
    project/nimbuildconfiguration.cpp project/nimbuildconfiguration.h
    project/nimcompilerbuildstep.cpp project/nimcompilerbuildstep.h
    project/nimcompilercleanstep.cpp project/nimcompilercleanstep.h
    project/nimoutputtaskparser.cpp project/nimoutputtaskparser.h
    project/nimproject.cpp project/nimproject.h
    project/nimrunconfiguration.cpp project/nimrunconfiguration.h
    project/nimtoolchain.cpp project/nimtoolchain.h
    project/nimtoolchainfactory.cpp project/nimtoolchainfactory.h
    settings/nimcodestylepreferencesfactory.cpp settings/nimcodestylepreferencesfactory.h
    settings/nimcodestylepreferenceswidget.cpp settings/nimcodestylepreferenceswidget.h settings/nimcodestylepreferenceswidget.ui
    settings/nimcodestylesettingspage.cpp settings/nimcodestylesettingspage.h
    settings/nimsettings.cpp settings/nimsettings.h
    settings/nimtoolssettingspage.cpp settings/nimtoolssettingspage.h
    settings/nimtoolssettingswidget.ui
    suggest/client.cpp suggest/client.h
    suggest/clientrequests.cpp suggest/clientrequests.h
    suggest/nimsuggest.cpp suggest/nimsuggest.h
    suggest/nimsuggestcache.cpp suggest/nimsuggestcache.h
    suggest/server.cpp suggest/server.h
    suggest/sexprlexer.h suggest/sexprparser.h
    tools/nimlexer.cpp tools/nimlexer.h
    tools/sourcecodestream.h
)
