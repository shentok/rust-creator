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

#include "nimbuildconfiguration.h"
#include "nimcompilerbuildstep.h"
#include "nimproject.h"
#include "nimbuildconfiguration.h"
#include "nimcompilerbuildstep.h"
#include "nimcompilercleanstep.h"
#include "nimproject.h"

#include "../nimconstants.h"

#include <projectexplorer/buildconfiguration.h>
#include <projectexplorer/buildinfo.h>
#include <projectexplorer/buildsteplist.h>
#include <projectexplorer/buildstep.h>
#include <projectexplorer/kit.h>
#include <projectexplorer/projectexplorer.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/projectmacroexpander.h>
#include <projectexplorer/target.h>
#include <projectexplorer/projectconfigurationaspects.h>
#include <utils/mimetypes/mimedatabase.h>
#include <utils/qtcassert.h>

using namespace ProjectExplorer;
using namespace Utils;

namespace Nim {

static FilePath defaultBuildDirectory(const Kit *k,
                                      const QString &projectFilePath,
                                      const QString &bc,
                                      BuildConfiguration::BuildType buildType)
{
    QFileInfo projectFileInfo(projectFilePath);

    ProjectMacroExpander expander(projectFilePath, projectFileInfo.baseName(), k, bc, buildType);
    QString buildDirectory = expander.expand(ProjectExplorerPlugin::buildDirectoryTemplate());

    if (FileUtils::isAbsolutePath(buildDirectory))
        return FilePath::fromString(buildDirectory);

    auto projectDir = FilePath::fromString(projectFileInfo.absoluteDir().absolutePath());
    return projectDir.pathAppended(buildDirectory);
}

NimBuildConfiguration::NimBuildConfiguration(Target *target, Core::Id id)
    : BuildConfiguration(target, id)
{
    setConfigWidgetDisplayName(tr("General"));
    setConfigWidgetHasFrame(true);
    setBuildDirectorySettingsKey("Nim.NimBuildConfiguration.BuildDirectory");
}

bool NimBuildConfiguration::fromMap(const QVariantMap &map)
{
    if (!ProjectExplorer::ProjectConfiguration::fromMap(map))
        return false;

    return ProjectExplorer::BuildConfiguration::fromMap(map);
}

void NimBuildConfiguration::initialize(const BuildInfo &info)
{
    BuildConfiguration::initialize(info);

    auto project = target()->project();
    QTC_ASSERT(project, return);

    // Create the build configuration and initialize it from build info
    setBuildDirectory(info.buildDirectory);

    // Add nim compiler build step
    {
        BuildStepList *buildSteps = stepList(ProjectExplorer::Constants::BUILDSTEPS_BUILD);
        auto nimCompilerBuildStep = new NimCompilerBuildStep(buildSteps);
        if (info.typeName == "build (debug)")
        {
            nimCompilerBuildStep->setUserCompilerOptions(QStringList{"build"});
        }
        else if (info.typeName == "build (release)")
        {
            nimCompilerBuildStep->setUserCompilerOptions(QStringList{"build", "--release"});
        }
        else if (info.typeName == "test (debug)")
        {
            nimCompilerBuildStep->setUserCompilerOptions(QStringList{"test"});
        }
        else if (info.typeName == "test (release)")
        {
            nimCompilerBuildStep->setUserCompilerOptions(QStringList{"test", "--release"});
        }
        buildSteps->appendStep(nimCompilerBuildStep);
    }

    // Add clean step
    {
        BuildStepList *cleanSteps = stepList(ProjectExplorer::Constants::BUILDSTEPS_CLEAN);
        cleanSteps->appendStep(new NimCompilerCleanStep(cleanSteps));
    }
}

BuildConfiguration::BuildType NimBuildConfiguration::buildType() const
{
    return BuildConfiguration::Unknown;
}


NimBuildConfigurationFactory::NimBuildConfigurationFactory()
{
    registerBuildConfiguration<NimBuildConfiguration>(Constants::C_NIMBUILDCONFIGURATION_ID);
    setSupportedProjectType(Constants::C_NIMPROJECT_ID);
    setSupportedProjectMimeTypeName(Constants::C_NIM_PROJECT_MIMETYPE);
}

QList<BuildInfo> NimBuildConfigurationFactory::availableBuilds(const Target *parent) const
{
    QList<BuildInfo> result;
    for (auto typeName : QStringList{"build (debug)", "build (release)", "test (debug)", "test (release)"})
        result.push_back(createBuildInfo(parent->kit(), typeName));
    return result;
}

QList<BuildInfo> NimBuildConfigurationFactory::availableSetups(const Kit *k, const QString &projectPath) const
{
    QList<BuildInfo> result;
    for (auto typeName : QStringList{"build (debug)", "build (release)", "test (debug)", "test (release)"}) {
        BuildInfo info = createBuildInfo(k, typeName);
        info.buildDirectory = defaultBuildDirectory(k, projectPath, info.typeName, info.buildType);
        result.push_back(info);
    }
    return result;
}

BuildInfo NimBuildConfigurationFactory::createBuildInfo(const Kit *k, const QString &typeName) const
{
    BuildInfo info(this);
    info.buildType = BuildConfiguration::Unknown;
    info.kitId = k->id();
    info.typeName = typeName;
    info.displayName = typeName;
    return info;
}

QString NimBuildConfigurationFactory::displayName(BuildConfiguration::BuildType buildType) const
{
    switch (buildType) {
    case ProjectExplorer::BuildConfiguration::Debug:
        return tr("Debug");
    case ProjectExplorer::BuildConfiguration::Profile:
        return tr("Profile");
    case ProjectExplorer::BuildConfiguration::Release:
        return tr("Release");
    default:
        return QString();
    }
}

} // namespace Nim

