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

#include "nimbuildconfigurationwidget.h"
#include "nimcompilerbuildstep.h"
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
                                      const FilePath &projectFilePath,
                                      const QString &bc,
                                      BuildConfiguration::BuildType buildType)
{
    QFileInfo projectFileInfo = projectFilePath.toFileInfo();

    ProjectMacroExpander expander(projectFilePath,
                                  projectFileInfo.baseName(), k, bc, buildType);
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

    appendInitialBuildStep(Constants::C_NIMCOMPILERBUILDSTEP_ID);
    appendInitialCleanStep(Constants::C_NIMCOMPILERCLEANSTEP_ID);

    setInitializer([this, target](const BuildInfo &info) {
        // Create the build configuration and initialize it from build info
        setBuildDirectory(defaultBuildDirectory(target->kit(),
                                                project()->projectFilePath(),
                                                displayName(),
                                                buildType()));

        auto buildStep = nimCompilerBuildStep();
        QTC_ASSERT(buildStep, return);
        auto cleanStep = nimCompilerCleanStep();
        QTC_ASSERT(cleanStep, return);
        DefaultBuildOptions defaultOption;
        switch (info.buildType) {
        case BuildConfiguration::Release:
            defaultOption = DefaultBuildOptions::Release;
            break;
        case BuildConfiguration::Debug:
            defaultOption = DefaultBuildOptions::Debug;
            break;
        default:
            defaultOption = DefaultBuildOptions::Empty;
            break;
        }
        setDefaultCompilerOptions(defaultOption);
        buildStep->setUserCompilerOptions(QStringList() << "build");
        cleanStep->setUserCompilerOptions(QStringList() << "clean");

        const Utils::FilePaths nimFiles = project()->files([](const Node *n) {
            return Project::AllFiles(n) && n->path().endsWith(".toml");
        });

        if (!nimFiles.isEmpty()) {
            setTargetNimFile(nimFiles.first());
        }
    });

    connect(this, &NimBuildConfiguration::targetNimFileChanged,
            this, &NimBuildConfiguration::processParametersChanged);
    connect(this, &NimBuildConfiguration::defaultCompilerOptionsChanged,
            this, &NimBuildConfiguration::processParametersChanged);
}


NamedWidget *NimBuildConfiguration::createConfigWidget()
{
    return new NimBuildConfigurationWidget(this);
}


bool NimBuildConfiguration::fromMap(const QVariantMap &map)
{
    m_defaultOptions = static_cast<DefaultBuildOptions>(map[Constants::C_NIMCOMPILERBUILDSTEP_DEFAULTBUILDOPTIONS].toInt());
    m_targetNimFile = FilePath::fromString(map[Constants::C_NIMCOMPILERBUILDSTEP_TARGETNIMFILE].toString());

    if (!ProjectExplorer::ProjectConfiguration::fromMap(map))
        return false;

    return ProjectExplorer::BuildConfiguration::fromMap(map);
}

QVariantMap NimBuildConfiguration::toMap() const
{
    QVariantMap result = BuildConfiguration::toMap();
    result[Constants::C_NIMCOMPILERBUILDSTEP_DEFAULTBUILDOPTIONS] = m_defaultOptions;
    result[Constants::C_NIMCOMPILERBUILDSTEP_TARGETNIMFILE] = m_targetNimFile.toString();
    return result;
}


NimBuildConfiguration::DefaultBuildOptions NimBuildConfiguration::defaultCompilerOptions() const
{
    return m_defaultOptions;
}


void NimBuildConfiguration::setDefaultCompilerOptions(const DefaultBuildOptions options)
{
    if (m_defaultOptions == options)
        return;
    m_defaultOptions = options;
    emit defaultCompilerOptionsChanged(options);
}


FilePath NimBuildConfiguration::targetNimFile() const
{
    return m_targetNimFile;
}


void NimBuildConfiguration::setTargetNimFile(const FilePath &targetNimFile)
{
    if (targetNimFile == m_targetNimFile)
        return;
    m_targetNimFile = targetNimFile;
    emit targetNimFileChanged(targetNimFile);
}


FilePath NimBuildConfiguration::outFilePath() const
{
    const NimCompilerBuildStep *step = nimCompilerBuildStep();
    QTC_ASSERT(step, return FilePath());
    const QString targetName = Utils::HostOsInfo::withExecutableSuffix(m_targetNimFile.toFileInfo().baseName());
    return buildDirectory().pathAppended(targetName);
}

void NimBuildConfiguration::updateTargetNimFile()
{
    if (!m_targetNimFile.isEmpty())
        return;
    const Utils::FilePaths nimFiles = project()->files([](const Node *n) {
        return Project::AllFiles(n) && n->path().endsWith(".toml");
    });
    if (!nimFiles.isEmpty())
        setTargetNimFile(nimFiles.at(0));
}

const NimCompilerBuildStep *NimBuildConfiguration::nimCompilerBuildStep() const
{
    foreach (const BuildStep *step, buildSteps()->steps())
        if (step->id() == Constants::C_NIMCOMPILERBUILDSTEP_ID)
            return qobject_cast<const NimCompilerBuildStep *>(step);
    return nullptr;
}

NimCompilerBuildStep *NimBuildConfiguration::nimCompilerBuildStep()
{
    foreach (BuildStep *step, buildSteps()->steps())
        if (step->id() == Constants::C_NIMCOMPILERBUILDSTEP_ID)
            return qobject_cast<NimCompilerBuildStep *>(step);
    return nullptr;
}

NimCompilerBuildStep *NimBuildConfiguration::nimCompilerCleanStep()
{
    foreach (BuildStep *step, cleanSteps()->steps())
        if (step->id() == Constants::C_NIMCOMPILERCLEANSTEP_ID)
            return qobject_cast<NimCompilerBuildStep *>(step);
    return nullptr;
}


NimBuildConfigurationFactory::NimBuildConfigurationFactory()
{
    registerBuildConfiguration<NimBuildConfiguration>(Constants::C_NIMBUILDCONFIGURATION_ID);
    setSupportedProjectType(Constants::C_NIMPROJECT_ID);
    setSupportedProjectMimeTypeName(Constants::C_NIM_PROJECT_MIMETYPE);

    setBuildGenerator([](const Kit *k, const FilePath &projectPath, bool forSetup) {
        const auto oneBuild = [&](BuildConfiguration::BuildType buildType, const QString &typeName) {
            BuildInfo info;
            info.buildType = buildType;
            info.typeName = typeName;
            if (forSetup) {
                info.displayName = info.typeName;
                info.buildDirectory = defaultBuildDirectory(k, projectPath, info.typeName, buildType);
            }
            return info;
        };
        return QList<BuildInfo>{
            oneBuild(BuildConfiguration::Debug, BuildConfiguration::tr("Debug")),
            oneBuild(BuildConfiguration::Release, BuildConfiguration::tr("Release"))
        };
    });
}

} // namespace Nim

