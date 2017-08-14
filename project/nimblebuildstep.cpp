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

#include "nimblebuildstep.h"
#include "nimblebuildstepwidget.h"
#include "nimbletaskstepwidget.h"
#include "nimconstants.h"
#include "nimbleproject.h"

#include <projectexplorer/buildconfiguration.h>
#include <projectexplorer/ioutputparser.h>
#include <projectexplorer/processparameters.h>
#include <projectexplorer/projectexplorerconstants.h>

#include <QRegularExpression>
#include <QStandardPaths>

using namespace Nim;
using namespace ProjectExplorer;
using namespace Utils;

namespace {

class NimParser : public OutputTaskParser
{
public:
    Result handleLine(const QString &line, Utils::OutputFormat) override
    {
        static const QRegularExpression error(QStringLiteral("^error(\\[E\\d+\\])?: "));
        static const QRegularExpression warning(QStringLiteral("^warning(\\[.\\d+\\])?: "));
        static const QRegularExpression location(QStringLiteral("^\\s*--> (.*):(\\d+):(\\d+)"));

        if (error.match(line).hasMatch()) {
            m_type = Task::Error;
            m_message += line;
            return Status::InProgress;
        }

        if (warning.match(line).hasMatch()) {
            m_type = Task::Warning;
            m_message += line;
            return Status::InProgress;
        }

        const QRegularExpressionMatch locationMatch = location.match(line);
        if (locationMatch.hasMatch()) {
            m_fileName = locationMatch.captured(1);
            m_lineNumber = locationMatch.captured(2).toInt();
//            const int rowNumber = locationMatch.captured(3).toInt();
            m_message += line;
            return Status::InProgress;
        }

        if (m_message.isEmpty()) {
            return Status::NotHandled;
        }

        if (!line.trimmed().isEmpty()) {
            m_message += line;
            return Status::InProgress;
        }

        const CompileTask t(m_type, m_message, absoluteFilePath(FilePath::fromUserInput(m_fileName)),
                            m_lineNumber);

        m_message.clear();
        m_fileName.clear();
        m_lineNumber = 0;
        m_type = Task::Unknown;

        LinkSpecs linkSpecs;
        addLinkSpecForAbsoluteFilePath(linkSpecs, t.file, t.line, locationMatch, 1);
        scheduleTask(t, 1);
        return {Status::Done, linkSpecs};
    }

private:
    QString m_message;
    QString m_fileName;
    int m_lineNumber = 0;
    Task::TaskType m_type = Task::TaskType::Unknown;
};

}

NimbleBuildStep::NimbleBuildStep(BuildStepList *parentList, Utils::Id id)
    : AbstractProcessStep(parentList, id)
{
    setDefaultDisplayName(tr(Constants::C_NIMBLEBUILDSTEP_DISPLAY));
    setDisplayName(tr(Constants::C_NIMBLEBUILDSTEP_DISPLAY));
    QTC_ASSERT(buildConfiguration(), return);
    QObject::connect(buildConfiguration(), &BuildConfiguration::buildTypeChanged, this, &NimbleBuildStep::resetArguments);
    QObject::connect(this, &NimbleBuildStep::argumentsChanged, this, &NimbleBuildStep::onArgumentsChanged);
    resetArguments();
}

bool NimbleBuildStep::init()
{
    ProcessParameters* params = processParameters();
    params->setEnvironment(buildEnvironment());
    params->setMacroExpander(macroExpander());
    params->setWorkingDirectory(project()->projectDirectory());
    params->setCommandLine({QStandardPaths::findExecutable("cargo"), QStringList("build") << m_arguments.split(QChar::Space)});
    return AbstractProcessStep::init();
}

void NimbleBuildStep::setupOutputFormatter(OutputFormatter *formatter)
{
    const auto parser = new NimParser();
    parser->addSearchDir(project()->projectDirectory());
    formatter->addLineParser(parser);
    AbstractProcessStep::setupOutputFormatter(formatter);
}

BuildStepConfigWidget *NimbleBuildStep::createConfigWidget()
{
    return new NimbleBuildStepWidget(this);
}

QString NimbleBuildStep::arguments() const
{
    return m_arguments;
}

void NimbleBuildStep::setArguments(const QString &args)
{
    if (m_arguments == args)
        return;
    m_arguments = args;
    emit argumentsChanged(args);
}

void NimbleBuildStep::resetArguments()
{
    setArguments(defaultArguments());
}

bool NimbleBuildStep::fromMap(const QVariantMap &map)
{
    m_arguments = map.value(Constants::C_NIMBLEBUILDSTEP_ARGUMENTS, defaultArguments()).toString();
    return AbstractProcessStep::fromMap(map);
}

QVariantMap NimbleBuildStep::toMap() const
{
    auto map = AbstractProcessStep::toMap();
    map[Constants::C_NIMBLEBUILDSTEP_ARGUMENTS] = m_arguments;
    return map;
}

QString NimbleBuildStep::defaultArguments() const
{
    switch (buildType()) {
    case BuildConfiguration::Debug:
        return {};
    case BuildConfiguration::Unknown:
    case BuildConfiguration::Profile:
    case BuildConfiguration::Release:
    default:
        return {"--release"};
    }
}

void NimbleBuildStep::onArgumentsChanged()
{
    ProcessParameters* params = processParameters();
    params->setCommandLine({QStandardPaths::findExecutable("cargo"), QStringList("build") << m_arguments.split(QChar::Space)});
}

NimbleBuildStepFactory::NimbleBuildStepFactory()
{
    registerStep<NimbleBuildStep>(Constants::C_NIMBLEBUILDSTEP_ID);
    setDisplayName(NimbleBuildStep::tr("Cargo Build"));
    setSupportedStepList(ProjectExplorer::Constants::BUILDSTEPS_BUILD);
    setSupportedConfiguration(Constants::C_NIMBLEBUILDCONFIGURATION_ID);
    setRepeatable(true);
}
