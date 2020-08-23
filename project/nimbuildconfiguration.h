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

#include <projectexplorer/buildconfiguration.h>
#include <projectexplorer/target.h>

namespace Nim {

class NimCompilerBuildStep;

class NimBuildConfiguration : public ProjectExplorer::BuildConfiguration
{
    Q_OBJECT

    friend class ProjectExplorer::BuildConfigurationFactory;
    NimBuildConfiguration(ProjectExplorer::Target *target, Utils::Id id);

    ProjectExplorer::NamedWidget *createConfigWidget() override;

    bool fromMap(const QVariantMap &map) override;

    QVariantMap toMap() const override;

public:
    enum NimBuildType { Default = 0, Debug, Release};

    NimBuildType nimBuildType() const;
    void setNimBuildType(NimBuildType buildType);

    Utils::FilePath targetNimFile() const;
    void setTargetNimFile(const Utils::FilePath &targetNimFile);

    Utils::FilePath outFilePath() const;

signals:
    void nimBuildTypeChanged(NimBuildType options);
    void targetNimFileChanged(const Utils::FilePath &targetNimFile);
    void processParametersChanged();

private:
    void updateTargetNimFile();
    const NimCompilerBuildStep *nimCompilerBuildStep() const;
    NimCompilerBuildStep *nimCompilerBuildStep();
    NimCompilerBuildStep *nimCompilerCleanStep();

    NimBuildType m_buildType;
    Utils::FilePath m_targetNimFile;
};


class NimBuildConfigurationFactory final : public ProjectExplorer::BuildConfigurationFactory
{
public:
    NimBuildConfigurationFactory();
};

} // Nim
