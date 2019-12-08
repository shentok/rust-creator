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

#include "nimcompilerbuildstepconfigwidget.h"
#include "ui_nimcompilerbuildstepconfigwidget.h"
#include "nimbuildconfiguration.h"
#include "nimcompilerbuildstep.h"
#include "nimproject.h"

#include "../nimconstants.h"

#include <projectexplorer/processparameters.h>

#include <utils/qtcassert.h>

using namespace ProjectExplorer;
using namespace Utils;

namespace Nim {

NimCompilerBuildStepConfigWidget::NimCompilerBuildStepConfigWidget(NimCompilerBuildStep *buildStep)
    : BuildStepConfigWidget(buildStep)
    , m_buildStep(buildStep)
    , m_ui(new Ui::NimCompilerBuildStepConfigWidget())
{
    m_ui->setupUi(this);

    setDisplayName(tr(Constants::C_NIMCOMPILERBUILDSTEPWIDGET_DISPLAY));
    setSummaryText(tr(Constants::C_NIMCOMPILERBUILDSTEPWIDGET_SUMMARY));

    // Connect the project signals
    auto project = static_cast<NimProject *>(m_buildStep->project());
    connect(project, &NimProject::fileListChanged,
            this, &NimCompilerBuildStepConfigWidget::updateUi);

    // Connect build step signals
    connect(m_buildStep, &NimCompilerBuildStep::processParametersChanged,
            this, &NimCompilerBuildStepConfigWidget::updateUi);

    // Connect UI signals
    connect(m_ui->targetComboBox, QOverload<int>::of(&QComboBox::activated),
            this, &NimCompilerBuildStepConfigWidget::onTargetChanged);
    connect(m_ui->additionalArgumentsLineEdit, &QLineEdit::textEdited,
            this, &NimCompilerBuildStepConfigWidget::onAdditionalArgumentsTextEdited);

    updateUi();
}

NimCompilerBuildStepConfigWidget::~NimCompilerBuildStepConfigWidget() = default;

void NimCompilerBuildStepConfigWidget::onTargetChanged(int index)
{
    Q_UNUSED(index);
    auto data = m_ui->targetComboBox->currentData();
    FilePath path = FilePath::fromString(data.toString());
}

void NimCompilerBuildStepConfigWidget::updateUi()
{
    updateCommandLineText();
    updateTargetComboBox();
    updateAdditionalArgumentsLineEdit();
}

void NimCompilerBuildStepConfigWidget::onAdditionalArgumentsTextEdited(const QString &text)
{
    m_buildStep->setUserCompilerOptions(text.split(QChar::Space));
}

void NimCompilerBuildStepConfigWidget::updateCommandLineText()
{
    ProcessParameters *parameters = m_buildStep->processParameters();

    QStringList command;
    command << parameters->command().toString();
    command << parameters->arguments();

    // Remove empty args
    auto predicate = [](const QString & str) { return str.isEmpty(); };
    auto it = std::remove_if(command.begin(), command.end(), predicate);
    command.erase(it, command.end());

    m_ui->commandTextEdit->setText(command.join(QChar::LineFeed));
}

void NimCompilerBuildStepConfigWidget::updateTargetComboBox()
{
    QTC_ASSERT(m_buildStep, return);

    auto project = qobject_cast<NimProject *>(m_buildStep->project());
    QTC_ASSERT(project, return);

    // Re enter the files
    m_ui->targetComboBox->clear();
    foreach (const FilePath &file, project->nimFiles())
        m_ui->targetComboBox->addItem(file.fileName(), file.toString());

    const int index = m_ui->targetComboBox->findData(project->projectFilePath().toString());
    m_ui->targetComboBox->setCurrentIndex(index);
}

void NimCompilerBuildStepConfigWidget::updateAdditionalArgumentsLineEdit()
{
    const QString text = m_buildStep->userCompilerOptions().join(QChar::Space);
    m_ui->additionalArgumentsLineEdit->setText(text);
}

}

