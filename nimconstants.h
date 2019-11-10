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

const char C_NIMPROJECT_ID[] = "Nim.NimProject";

// NimToolChain
const char C_NIMTOOLCHAIN_TYPEID[] = "Nim.NimToolChain";
const char C_NIMTOOLCHAIN_COMPILER_COMMAND_KEY[] = "Nim.NimToolChain.CompilerCommand";

// NimProject
const char C_NIMPROJECT_EXCLUDEDFILES[] = "Nim.NimProjectExcludedFiles";

// NimBuildConfiguration
const char C_NIMBUILDCONFIGURATION_ID[] = "Nim.NimBuildConfiguration";

// NimCompilerBuildStep
const char C_NIMCOMPILERBUILDSTEP_ID[] = "Nim.NimCompilerBuildStep";
const char C_NIMCOMPILERBUILDSTEP_DISPLAY[] = QT_TRANSLATE_NOOP("NimCompilerBuildStep", "Nim Compiler Build Step");
const QString C_NIMCOMPILERBUILDSTEP_USERCOMPILEROPTIONS = QStringLiteral("Nim.NimCompilerBuildStep.UserCompilerOptions");
const QString C_NIMCOMPILERBUILDSTEP_DEFAULTBUILDOPTIONS = QStringLiteral("Nim.NimCompilerBuildStep.DefaultBuildOptions");
const QString C_NIMCOMPILERBUILDSTEP_TARGETNIMFILE = QStringLiteral("Nim.NimCompilerBuildStep.TargetNimFile");

// NimCompilerBuildStepWidget
const char C_NIMCOMPILERBUILDSTEPWIDGET_DISPLAY[] = QT_TRANSLATE_NOOP("NimCompilerBuildStepConfigWidget", "Nim build step");
const char C_NIMCOMPILERBUILDSTEPWIDGET_SUMMARY[] = QT_TRANSLATE_NOOP("NimCompilerBuildStepConfigWidget", "Nim build step");

// NimCompilerCleanStep
const char C_NIMCOMPILERCLEANSTEP_ID[] = "Nim.NimCompilerCleanStep";

const char C_NIMLANGUAGE_ID[] = "Nim";
const char C_NIMLANGUAGE_NAME[] = QT_TRANSLATE_NOOP("NimCodeStylePreferencesFactory", "Nim");

/*******************************************************************************
 * MIME type
 ******************************************************************************/
const char C_NIM_MIMETYPE[] = "text/x-nim";
const char C_NIM_PROJECT_MIMETYPE[] = "text/x-nim-project";

}
}
