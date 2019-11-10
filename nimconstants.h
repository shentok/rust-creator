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

const char C_NIMBLEPROJECT_ID[] = "Nim.NimbleProject";

// NimToolChain
const char C_NIMTOOLCHAIN_TYPEID[] = "Nim.NimToolChain";

// NimBuildConfiguration
const char C_NIMBLEBUILDCONFIGURATION_ID[] = "Nim.NimbleBuildConfiguration";
const char C_NIMBLEBUILDCONFIGURATION_BUILDTYPE[] = "Nim.NimbleBuildConfiguration.BuildType";

// NimbleBuildStep
const char C_NIMBLEBUILDSTEP_ID[] = "Nim.NimbleBuildStep";
const char C_NIMBLEBUILDSTEP_ARGUMENTS[] = "Nim.NimbleBuildStep.Arguments";

// NimbleTaskStep
const char C_NIMBLETASKSTEP_ID[] = "Nim.NimbleTaskStep";
const char C_NIMBLETASKSTEP_DISPLAY[] = QT_TRANSLATE_NOOP("NimbleTaskStep", "Nimble Task");
const QString C_NIMBLETASKSTEP_TASKNAME = QStringLiteral("Nim.NimbleTaskStep.TaskName");
const QString C_NIMBLETASKSTEP_TASKARGS = QStringLiteral("Nim.NimbleTaskStep.TaskArgs");

const char C_NIMLANGUAGE_ID[] = "Nim";
const char C_NIMLANGUAGE_NAME[] = QT_TRANSLATE_NOOP("NimCodeStylePreferencesFactory", "Nim");

/*******************************************************************************
 * MIME type
 ******************************************************************************/
const char C_NIM_MIMETYPE[] = "text/x-nim";
const char C_NIMBLE_MIMETYPE[] = "text/x-nimble";

}
}
