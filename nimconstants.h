/****************************************************************************
**
** Copyright (C) Filippo Cucchetto <filippocucchetto@gmail.com>
** Contact: http://www.qt.io/licensing
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
****************************************************************************/

#pragma once

#include <QObject>
#include <QtGlobal>

namespace Nim {
namespace Constants {

const char C_NIMPROJECT_ID[] = "Rust.RustProject";
const char C_NIMEDITOR_ID[] = "Rust.RustEditor";
const char C_EDITOR_DISPLAY_NAME[] = QT_TRANSLATE_NOOP("OpenWith::Editors", "Rust Editor");
const char C_NIM_ICON_PATH[] = ":/images/nim.png";

// NimToolChain
const char C_NIMTOOLCHAIN_TYPEID[] = "Rust.RustToolChain";
const char C_NIMTOOLCHAIN_COMPILER_COMMAND_KEY[] = "Rust.RustToolChain.CompilerCommand";

// NimRunConfiguration
const char C_NIMRUNCONFIGURATION_ID[] = "Rust.RustRunConfiguration";
const char C_NIMRUNCONFIGURATION_DISPLAY[] = QT_TRANSLATE_NOOP("RustRunConfiguration", "Current Build Target");
const char C_NIMRUNCONFIGURATION_DEFAULT_DISPLAY[] = QT_TRANSLATE_NOOP("RustRunConfiguration", "Current Build Target");
const QString C_NIMRUNCONFIGURATION_EXECUTABLE_KEY = QStringLiteral("Rust.RustRunConfiguration.Executable");
const QString C_NIMRUNCONFIGURATION_WORKINGDIRECTORY_KEY = QStringLiteral("Rust.RustRunConfiguration.WorkingDirectory");
const QString C_NIMRUNCONFIGURATION_COMMANDLINEARGS_KEY = QStringLiteral("Rust.RustRunConfiguration.CommandlineArgs");
const QString C_NIMRUNCONFIGURATION_RUNMODE_KEY = QStringLiteral("Rust.RustRunConfiguration.RunMode");
const QString C_NIMRUNCONFIGURATION_WORKINGDIRECTORYASPECT_ID = QStringLiteral("Rust.RustRunConfiguration.WorkingDirectoryAspect");
const QString C_NIMRUNCONFIGURATION_ARGUMENTASPECT_ID = QStringLiteral("Rust.RustRunConfiguration.ArgumentAspect");
const QString C_NIMRUNCONFIGURATION_TERMINALASPECT_ID = QStringLiteral("Rust.RustRunConfiguration.TerminalAspect");

// NimProject
const char C_NIMPROJECT_EXCLUDEDFILES[] = "Rust.RustProjectExcludedFiles";

// NimBuildConfiguration
const char C_NIMBUILDCONFIGURATION_ID[] = "Rust.RustBuildConfiguration";
const QString C_NIMBUILDCONFIGURATION_DISPLAY_KEY = QStringLiteral("Rust.RustBuildConfiguration.Display");
const QString C_NIMBUILDCONFIGURATION_BUILDDIRECTORY_KEY = QStringLiteral("Rust.RustBuildConfiguration.BuildDirectory");

// NimBuildConfigurationWidget
const char C_NIMBUILDCONFIGURATIONWIDGET_DISPLAY[] = QT_TRANSLATE_NOOP("RustBuildConfigurationWidget","General");

// NimCompilerBuildStep
const char C_NIMCOMPILERBUILDSTEP_ID[] = "Rust.RustCompilerBuildStep";
const char C_NIMCOMPILERBUILDSTEP_DISPLAY[] = QT_TRANSLATE_NOOP("RustCompilerBuildStep", "Rust Compiler Build Step");
const QString C_NIMCOMPILERBUILDSTEP_USERCOMPILEROPTIONS = QStringLiteral("Rust.RustCompilerBuildStep.UserCompilerOptions");
const QString C_NIMCOMPILERBUILDSTEP_DEFAULTBUILDOPTIONS = QStringLiteral("Rust.RustCompilerBuildStep.DefaultBuildOptions");
const QString C_NIMCOMPILERBUILDSTEP_TARGETNIMFILE = QStringLiteral("Rust.RustCompilerBuildStep.TargetRustFile");

// NimCompilerBuildStepWidget
const char C_NIMCOMPILERBUILDSTEPWIDGET_DISPLAY[] = QT_TRANSLATE_NOOP("RustCompilerBuildStepConfigWidget", "Rust build step");
const char C_NIMCOMPILERBUILDSTEPWIDGET_SUMMARY[] = QT_TRANSLATE_NOOP("RustCompilerBuildStepConfigWidget", "Rust build step");

// NimCompilerCleanStep
const char C_NIMCOMPILERCLEANSTEP_ID[] = "Rust.RustCompilerCleanStep";
const char C_NIMCOMPILERCLEANSTEP_DISPLAY[] = QT_TRANSLATE_NOOP("RustCompilerCleanStepFactory", "Rust Compiler Clean Step");

// NimCompilerCleanStepWidget
const char C_NIMCOMPILERCLEANSTEPWIDGET_DISPLAY[] = QT_TRANSLATE_NOOP("RustCompilerCleanStepWidget", "Rust clean step");
const char C_NIMCOMPILERCLEANSTEPWIDGET_SUMMARY[] = QT_TRANSLATE_NOOP("RustCompilerCleanStepWidget", "Rust clean step");

const char C_NIMLANGUAGE_ID[] = "Rust";
const char C_NIMCODESTYLESETTINGSPAGE_ID[] = "Rust.RustCodeStyleSettings";
const char C_NIMCODESTYLESETTINGSPAGE_DISPLAY[] = QT_TRANSLATE_NOOP("RustCodeStyleSettingsPage", "Code Style");
const char C_NIMCODESTYLESETTINGSPAGE_CATEGORY[] = "Z.Rust";
const char C_NIMCODESTYLESETTINGSPAGE_CATEGORY_DISPLAY[] = QT_TRANSLATE_NOOP("RustCodeStyleSettingsPage", "Rust");
const char C_NIMLANGUAGE_NAME[] = QT_TRANSLATE_NOOP("RustCodeStylePreferencesFactory", "Rust");
const char C_NIMGLOBALCODESTYLE_ID[] = "RustGlobal";
const QString C_NIMSNIPPETSGROUP_ID = QStringLiteral("Rust.RustSnippetsGroup");

const char C_NIMCODESTYLEPREVIEWSNIPPET[] =
        "import os\n"
        "\n"
        "type Foo = ref object of RootObj\n"
        "  name: string\n"
        "  value: int \n"
        "\n"
        "proc newFoo(): Foo =\n"
        "  new(result)\n"
        "\n"
        "if isMainModule():\n"
        "  let foo = newFoo()\n"
        "  echo foo.name\n";

/*******************************************************************************
 * MIME type
 ******************************************************************************/
const char C_NIM_MIMETYPE[] = "text/x-rustsrc";
const char C_NIM_MIME_ICON[] = "text-x-nim";
const char C_NIM_PROJECT_MIMETYPE[] = "text/x-cargo.toml";

}
}
