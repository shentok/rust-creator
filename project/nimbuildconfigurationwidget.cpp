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

#include "nimbuildconfigurationwidget.h"
#include "nimbuildconfiguration.h"
#include "nimbuildsystem.h"
#include "nimcompilerbuildstep.h"

#include "ui_nimbuildconfigurationwidget.h"

#include "../nimconstants.h"

#include <projectexplorer/processparameters.h>

#include <utils/qtcassert.h>
#include <utils/qtcprocess.h>

using namespace ProjectExplorer;
using namespace Utils;

namespace Nim {

NimBuildConfigurationWidget::NimBuildConfigurationWidget(NimBuildConfiguration *buildConfiguration)
    : ProjectExplorer::NamedWidget("General")
    , m_buildConfiguration(buildConfiguration)
    , m_ui(new Ui::NimBuildConfigurationWidget())
{
    m_ui->setupUi(this);

    // Connect the project signals
    connect(m_buildConfiguration->project(),
            &Project::fileListChanged,
            this,
            &NimBuildConfigurationWidget::updateUi);

    // Connect build step signals
    connect(m_buildConfiguration, &NimBuildConfiguration::processParametersChanged,
            this, &NimBuildConfigurationWidget::updateUi);

    // Connect UI signals
    connect(m_ui->targetComboBox, QOverload<int>::of(&QComboBox::activated),
            this, &NimBuildConfigurationWidget::onTargetChanged);
    connect(m_ui->defaultArgumentsComboBox, QOverload<int>::of(&QComboBox::activated),
            this, &NimBuildConfigurationWidget::onDefaultArgumentsComboBoxIndexChanged);

    updateUi();
}

NimBuildConfigurationWidget::~NimBuildConfigurationWidget() = default;

void NimBuildConfigurationWidget::onTargetChanged(int index)
{
    Q_UNUSED(index)
    auto data = m_ui->targetComboBox->currentData();
    FilePath path = FilePath::fromString(data.toString());
    m_buildConfiguration->setTargetNimFile(path);
}

void NimBuildConfigurationWidget::onDefaultArgumentsComboBoxIndexChanged(int index)
{
    auto options = static_cast<NimBuildConfiguration::DefaultBuildOptions>(index);
    m_buildConfiguration->setDefaultCompilerOptions(options);
}

void NimBuildConfigurationWidget::updateUi()
{
    updateTargetComboBox();
    updateDefaultArgumentsComboBox();
}

void NimBuildConfigurationWidget::updateTargetComboBox()
{
    QTC_ASSERT(m_buildConfiguration, return );

    // Re enter the files
    m_ui->targetComboBox->clear();

    const FilePaths nimFiles = m_buildConfiguration->project()->files([](const Node *n) {
        return Project::AllFiles(n) && n->path().endsWith(".toml");
    });

    for (const FilePath &file : nimFiles)
        m_ui->targetComboBox->addItem(file.fileName(), file.toString());

    const int index = m_ui->targetComboBox->findData(m_buildConfiguration->targetNimFile().toString());
    m_ui->targetComboBox->setCurrentIndex(index);
}

void NimBuildConfigurationWidget::updateDefaultArgumentsComboBox()
{
    const int index = m_buildConfiguration->defaultCompilerOptions();
    m_ui->defaultArgumentsComboBox->setCurrentIndex(index);
}

}

