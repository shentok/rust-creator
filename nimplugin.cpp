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

#include "nimplugin.h"

#include "nimconstants.h"
#include "project/nimbuildconfiguration.h"
#include "project/nimcompilerbuildstep.h"
#include "project/nimproject.h"
#include "project/nimrunconfiguration.h"
#include "project/nimtoolchainfactory.h"
#include "settings/nimsettings.h"

#include <coreplugin/fileiconprovider.h>
#include <projectexplorer/projectmanager.h>
#include <projectexplorer/toolchainmanager.h>
#include <projectexplorer/runcontrol.h>

using namespace Utils;
using namespace ProjectExplorer;

namespace Nim {

class NimPluginPrivate
{
public:
    NimSettings settings;
    NimBuildConfigurationFactory buildConfigFactory;
    NimRunConfigurationFactory nimRunConfigFactory;
    RunWorkerFactory nimRunWorkerFactory {
        RunWorkerFactory::make<SimpleTargetRunner>(),
        {ProjectExplorer::Constants::NORMAL_RUN_MODE},
        {nimRunConfigFactory.id()}
    };
    NimCompilerBuildStepFactory buildStepFactory;
    NimCompilerCleanStepFactory cleanStepFactory;
    NimToolChainFactory toolChainFactory;
};

RustPlugin::~RustPlugin()
{
    delete d;
}

bool RustPlugin::initialize(const QStringList &arguments, QString *errorMessage)
{
    Q_UNUSED(arguments)
    Q_UNUSED(errorMessage)

    d = new NimPluginPrivate;

    ToolChainManager::registerLanguage(Constants::C_NIMLANGUAGE_ID, Constants::C_NIMLANGUAGE_NAME);

    ProjectManager::registerProjectType<NimProject>(Constants::C_NIM_PROJECT_MIMETYPE);

    return true;
}

void RustPlugin::extensionsInitialized()
{
    // Add MIME overlay icons (these icons displayed at Project dock panel)
    const QIcon icon = Utils::Icon({{":/nim/images/settingscategory_nim.png",
            Utils::Theme::PanelTextColorDark
        }}, Utils::Icon::Tint).icon();
    if (!icon.isNull()) {
        Core::FileIconProvider::registerIconOverlayForMimeType(icon, Constants::C_NIM_MIMETYPE);
    }
}

} // namespace Nim
