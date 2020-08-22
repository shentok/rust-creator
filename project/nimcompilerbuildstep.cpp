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

#include "nimcompilerbuildstep.h"
#include "nimbuildconfiguration.h"
#include "nimbuildsystem.h"
#include "nimcompilerbuildstepconfigwidget.h"
#include "nimconstants.h"
#include "nimtoolchain.h"

#include <projectexplorer/buildconfiguration.h>
#include <projectexplorer/ioutputparser.h>
#include <projectexplorer/kitinformation.h>
#include <projectexplorer/processparameters.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <utils/qtcassert.h>

#include <QDir>
#include <QRegularExpression>

using namespace ProjectExplorer;
using namespace Utils;

namespace Nim {

class NimParser : public ProjectExplorer::OutputTaskParser
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

NimCompilerBuildStep::NimCompilerBuildStep(BuildStepList *parentList, Utils::Id id)
    : AbstractProcessStep(parentList, id)
{
    setDefaultDisplayName(tr(Constants::C_NIMCOMPILERBUILDSTEP_DISPLAY));
    setDisplayName(tr(Constants::C_NIMCOMPILERBUILDSTEP_DISPLAY));

    auto bc = qobject_cast<NimBuildConfiguration *>(buildConfiguration());
    connect(bc, &NimBuildConfiguration::buildDirectoryChanged,
            this, &NimCompilerBuildStep::updateProcessParameters);
    connect(bc, &BuildConfiguration::environmentChanged,
            this, &NimCompilerBuildStep::updateProcessParameters);
    connect(bc, &NimBuildConfiguration::processParametersChanged,
            this, &NimCompilerBuildStep::updateProcessParameters);
    updateProcessParameters();
}

void NimCompilerBuildStep::setupOutputFormatter(OutputFormatter *formatter)
{
    formatter->addLineParser(new NimParser);
    formatter->addLineParsers(target()->kit()->createOutputParsers());
    formatter->addSearchDir(processParameters()->effectiveWorkingDirectory());
    AbstractProcessStep::setupOutputFormatter(formatter);
}

BuildStepConfigWidget *NimCompilerBuildStep::createConfigWidget()
{
    auto widget = new NimCompilerBuildStepConfigWidget(this);
    if (id() == Constants::C_NIMCOMPILERBUILDSTEP_ID)
        widget->setDisplayName(tr(Constants::C_NIMCOMPILERBUILDSTEPWIDGET_DISPLAY));
    else
        widget->setDisplayName(tr(Constants::C_NIMCOMPILERCLEANSTEPWIDGET_DISPLAY));
    return widget;
}

bool NimCompilerBuildStep::fromMap(const QVariantMap &map)
{
    AbstractProcessStep::fromMap(map);
    m_userCompilerOptions = map[Constants::C_NIMCOMPILERBUILDSTEP_USERCOMPILEROPTIONS].toString().split('|');
    updateProcessParameters();
    return true;
}

QVariantMap NimCompilerBuildStep::toMap() const
{
    QVariantMap result = AbstractProcessStep::toMap();
    result[Constants::C_NIMCOMPILERBUILDSTEP_USERCOMPILEROPTIONS] = m_userCompilerOptions.join('|');
    return result;
}

QStringList NimCompilerBuildStep::userCompilerOptions() const
{
    return m_userCompilerOptions;
}

void NimCompilerBuildStep::setUserCompilerOptions(const QStringList &options)
{
    m_userCompilerOptions = options;
    emit userCompilerOptionsChanged(options);
    updateProcessParameters();
}

void NimCompilerBuildStep::updateProcessParameters()
{
    updateCommand();
    updateWorkingDirectory();
    updateEnvironment();
    emit processParametersChanged();
}

void NimCompilerBuildStep::updateWorkingDirectory()
{
    processParameters()->setWorkingDirectory(buildDirectory());
}

void NimCompilerBuildStep::updateCommand()
{
    auto bc = qobject_cast<NimBuildConfiguration *>(buildConfiguration());
    QTC_ASSERT(bc, return);

    QTC_ASSERT(target(), return);
    QTC_ASSERT(target()->kit(), return);
    Kit *kit = target()->kit();
    auto tc = dynamic_cast<NimToolChain*>(ToolChainKitAspect::toolChain(kit, Constants::C_NIMLANGUAGE_ID));
    QTC_ASSERT(tc, return);

    CommandLine cmd{tc->compilerCommand()};

    for (const QString &arg : m_userCompilerOptions) {
        if (!arg.isEmpty())
            cmd.addArg(arg);
    }

    if (bc->defaultCompilerOptions() == NimBuildConfiguration::Release)
        cmd.addArg("--release");

    cmd.addArg("--target-dir=" + bc->buildDirectory().toString());
    cmd.addArg("--manifest-path=" + bc->project()->projectFilePath().toString());

    processParameters()->setCommandLine(cmd);
}

void NimCompilerBuildStep::updateEnvironment()
{
    processParameters()->setEnvironment(buildEnvironment());
}

// NimCompilerBuildStepFactory

NimCompilerBuildStepFactory::NimCompilerBuildStepFactory()
{
    registerStep<NimCompilerBuildStep>(Constants::C_NIMCOMPILERBUILDSTEP_ID);
    setDisplayName(NimCompilerBuildStep::tr("Rust Compiler Build Step"));
    setSupportedStepList(ProjectExplorer::Constants::BUILDSTEPS_BUILD);
    setSupportedConfiguration(Constants::C_NIMBUILDCONFIGURATION_ID);
    setRepeatable(false);
}

// NimCompilerCleanStepFactory

NimCompilerCleanStepFactory::NimCompilerCleanStepFactory()
{
    registerStep<NimCompilerBuildStep>(Constants::C_NIMCOMPILERCLEANSTEP_ID);
    setDisplayName(NimCompilerBuildStep::tr("Rust Compiler Clean Step"));
    setSupportedStepList(ProjectExplorer::Constants::BUILDSTEPS_CLEAN);
    setSupportedConfiguration(Constants::C_NIMBUILDCONFIGURATION_ID);
    setRepeatable(false);
}

} // namespace Nim

#ifdef WITH_TESTS

#include "nimplugin.h"

#include <projectexplorer/outputparser_test.h>

#include <QTest>

namespace Nim {

void NimPlugin::testNimParser_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<OutputParserTester::Channel>("inputChannel");
    QTest::addColumn<QString>("childStdOutLines");
    QTest::addColumn<QString>("childStdErrLines");
    QTest::addColumn<Tasks >("tasks");
    QTest::addColumn<QString>("outputLines");

    // negative tests
    QTest::newRow("pass-through stdout")
            << "Sometext" << OutputParserTester::STDOUT
            << "Sometext\n" << QString()
            << Tasks()
            << QString();
    QTest::newRow("pass-through stderr")
            << "Sometext" << OutputParserTester::STDERR
            << QString() << "Sometext\n"
            << Tasks()
            << QString();

    // positive tests
    QTest::newRow("Parse error string")
            << QString::fromLatin1("main.nim(23, 1) Error: undeclared identifier: 'x'")
            << OutputParserTester::STDERR
            << QString() << QString()
            << Tasks({CompileTask(Task::Error,
                                  "Error: undeclared identifier: 'x'",
                                  FilePath::fromUserInput("main.nim"), 23)})
            << QString();

    QTest::newRow("Parse warning string")
            << QString::fromLatin1("lib/pure/parseopt.nim(56, 34) Warning: quoteIfContainsWhite is deprecated [Deprecated]")
            << OutputParserTester::STDERR
            << QString() << QString()
            << Tasks({CompileTask(Task::Warning,
                                  "Warning: quoteIfContainsWhite is deprecated [Deprecated]",
                                   FilePath::fromUserInput("lib/pure/parseopt.nim"), 56)})
            << QString();
}

void NimPlugin::testNimParser()
{
    OutputParserTester testbench;
    testbench.addLineParser(new NimParser);
    QFETCH(QString, input);
    QFETCH(OutputParserTester::Channel, inputChannel);
    QFETCH(Tasks, tasks);
    QFETCH(QString, childStdOutLines);
    QFETCH(QString, childStdErrLines);
    QFETCH(QString, outputLines);

    testbench.testParsing(input, inputChannel,
                          tasks, childStdOutLines, childStdErrLines,
                          outputLines);
}

}
#endif
