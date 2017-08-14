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
#include "nimconstants.h"
#include "nimcompilerbuildstepconfigwidget.h"
#include "nimproject.h"
#include "nimtoolchain.h"

#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/buildconfiguration.h>
#include <projectexplorer/kitinformation.h>
#include <projectexplorer/ioutputparser.h>
#include <utils/qtcassert.h>

#include <QDir>
#include <QRegularExpression>

using namespace ProjectExplorer;
using namespace Utils;

namespace Nim {

class LineStateMachine
{
public:
    LineStateMachine() :
        m_message(),
        m_fileName(),
        m_lineNumber(0),
        m_type(Task::Unknown)
    {
    }

    Task addLine(const QString &line)
    {
        static const QRegularExpression error(QStringLiteral("error(\\[E\\d+\\])?: "));
        static const QRegularExpression warning(QStringLiteral("warning(\\[.\\d+\\])?: "));
        static const QRegularExpression location(QStringLiteral("^--> (.*):(\\d+):(\\d+)"));

        if (error.match(line).hasMatch()) {
            m_type = Task::Error;
            m_message += line;
            return Task();
        }

        if (warning.match(line).hasMatch()) {
            m_type = Task::Warning;
            m_message += line;
            return Task();
        }

        const QRegularExpressionMatch locationMatch = location.match(line);
        if (locationMatch.hasMatch()) {
            m_fileName = locationMatch.captured(1);
            m_lineNumber = locationMatch.captured(2).toInt();
//            const int rowNumber = locationMatch.captured(3).toInt();
            m_message += '\n' + line;
            return Task();
        }

        if (m_message.isEmpty()) {
            return Task();
        }

        if (!line.isEmpty()) {
            m_message += '\n' + line;
            return Task();
        }

        const Task task(m_type,
                        m_message,
                        Utils::FileName::fromUserInput(m_fileName),
                        m_lineNumber,
                        ProjectExplorer::Constants::TASK_CATEGORY_COMPILE);

        m_message.clear();
        m_fileName.clear();
        m_lineNumber = 0;
        m_type = Task::Unknown;

        return task;
    }

private:
    QString m_message;
    QString m_fileName;
    int m_lineNumber;
    Task::TaskType m_type;
};

class NimParser : public ProjectExplorer::IOutputParser
{
public:
    NimParser() :
        m_stdOutput(),
        m_stdError()
    {
    }

    void stdOutput(const QString &line) final
    {
        const Task task = m_stdOutput.addLine(line.trimmed());

        if (!task.isNull()) {
            emit addTask(task);
        }

        IOutputParser::stdOutput(line);
    }

    void stdError(const QString &line) final
    {
        const Task task = m_stdError.addLine(line.trimmed());

        if (!task.isNull()) {
            emit addTask(task);
        }

        IOutputParser::stdError(line);
    }

private:
    LineStateMachine m_stdOutput;
    LineStateMachine m_stdError;
};

NimCompilerBuildStep::NimCompilerBuildStep(BuildStepList *parentList)
    : AbstractProcessStep(parentList, Constants::C_NIMCOMPILERBUILDSTEP_ID)
{
    setDefaultDisplayName(tr(Constants::C_NIMCOMPILERBUILDSTEP_DISPLAY));
    setDisplayName(tr(Constants::C_NIMCOMPILERBUILDSTEP_DISPLAY));

    auto bc = qobject_cast<NimBuildConfiguration *>(buildConfiguration());
    connect(bc, &NimBuildConfiguration::buildDirectoryChanged,
            this, &NimCompilerBuildStep::updateProcessParameters);
    connect(this, &NimCompilerBuildStep::outFilePathChanged,
            bc, &NimBuildConfiguration::outFilePathChanged);
    connect(bc->target()->project(), &ProjectExplorer::Project::fileListChanged,
            this, &NimCompilerBuildStep::updateTargetNimFile);
    updateProcessParameters();
}

bool NimCompilerBuildStep::init(QList<const BuildStep *> &earlierSteps)
{
    setOutputParser(new NimParser());
    if (IOutputParser *parser = target()->kit()->createOutputParser())
        appendOutputParser(parser);
    outputParser()->setWorkingDirectory(processParameters()->effectiveWorkingDirectory());
    return AbstractProcessStep::init(earlierSteps);
}

BuildStepConfigWidget *NimCompilerBuildStep::createConfigWidget()
{
    return new NimCompilerBuildStepConfigWidget(this);
}

bool NimCompilerBuildStep::fromMap(const QVariantMap &map)
{
    AbstractProcessStep::fromMap(map);
    m_userCompilerOptions = map[Constants::C_NIMCOMPILERBUILDSTEP_USERCOMPILEROPTIONS].toString().split(QLatin1Char('|'));
    m_defaultOptions = static_cast<DefaultBuildOptions>(map[Constants::C_NIMCOMPILERBUILDSTEP_DEFAULTBUILDOPTIONS].toInt());
    m_targetNimFile = FileName::fromString(map[Constants::C_NIMCOMPILERBUILDSTEP_TARGETNIMFILE].toString());
    updateProcessParameters();
    return true;
}

QVariantMap NimCompilerBuildStep::toMap() const
{
    QVariantMap result = AbstractProcessStep::toMap();
    result[Constants::C_NIMCOMPILERBUILDSTEP_USERCOMPILEROPTIONS] = m_userCompilerOptions.join(QLatin1Char('|'));
    result[Constants::C_NIMCOMPILERBUILDSTEP_DEFAULTBUILDOPTIONS] = m_defaultOptions;
    result[Constants::C_NIMCOMPILERBUILDSTEP_TARGETNIMFILE] = m_targetNimFile.toString();
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

NimCompilerBuildStep::DefaultBuildOptions NimCompilerBuildStep::defaultCompilerOptions() const
{
    return m_defaultOptions;
}

void NimCompilerBuildStep::setDefaultCompilerOptions(NimCompilerBuildStep::DefaultBuildOptions options)
{
    if (m_defaultOptions == options)
        return;
    m_defaultOptions = options;
    emit defaultCompilerOptionsChanged(options);
    updateProcessParameters();
}

FileName NimCompilerBuildStep::targetNimFile() const
{
    return m_targetNimFile;
}

void NimCompilerBuildStep::setTargetNimFile(const FileName &targetNimFile)
{
    if (targetNimFile == m_targetNimFile)
        return;
    m_targetNimFile = targetNimFile;
    emit targetNimFileChanged(targetNimFile);
    updateProcessParameters();
}

FileName NimCompilerBuildStep::outFilePath() const
{
    return m_outFilePath;
}

void NimCompilerBuildStep::setOutFilePath(const FileName &outFilePath)
{
    if (outFilePath == m_outFilePath)
        return;
    m_outFilePath = outFilePath;
    emit outFilePathChanged(outFilePath);
}

void NimCompilerBuildStep::updateProcessParameters()
{
    updateOutFilePath();
    updateCommand();
    updateArguments();
    updateWorkingDirectory();
    updateEnvironment();
    emit processParametersChanged();
}

void NimCompilerBuildStep::updateOutFilePath()
{
    auto bc = qobject_cast<NimBuildConfiguration *>(buildConfiguration());
    QTC_ASSERT(bc, return);
    const QString targetName = Utils::HostOsInfo::withExecutableSuffix(m_targetNimFile.toFileInfo().baseName());
    FileName outFilePath = bc->buildDirectory().appendPath(targetName);
    setOutFilePath(outFilePath);
}

void NimCompilerBuildStep::updateCommand()
{
    QTC_ASSERT(target(), return);
    QTC_ASSERT(target()->kit(), return);
    Kit *kit = target()->kit();
    auto tc = dynamic_cast<NimToolChain*>(ToolChainKitInformation::toolChain(kit, Constants::C_NIMLANGUAGE_ID));
    QTC_ASSERT(tc, return);
    processParameters()->setCommand(tc->compilerCommand().toString());
}

void NimCompilerBuildStep::updateWorkingDirectory()
{
    auto bc = qobject_cast<NimBuildConfiguration *>(buildConfiguration());
    QTC_ASSERT(bc, return);
    processParameters()->setWorkingDirectory(bc->buildDirectory().toString());
}

void NimCompilerBuildStep::updateArguments()
{
    auto bc = qobject_cast<NimBuildConfiguration *>(buildConfiguration());
    QTC_ASSERT(bc, return);

    QStringList arguments;
    arguments << QStringLiteral("build");

    switch (m_defaultOptions) {
    case Release:
        arguments << QStringLiteral("--release");
        break;
    case Debug:
        break;
    default:
        break;
    }

    arguments << m_userCompilerOptions;
    arguments << QStringLiteral("--manifest-path=\"%1\"").arg(m_targetNimFile.toString());

    // Remove empty args
    auto predicate = [](const QString &str) { return str.isEmpty(); };
    auto it = std::remove_if(arguments.begin(), arguments.end(), predicate);
    arguments.erase(it, arguments.end());

    processParameters()->setArguments(arguments.join(QChar::Space));
}

void NimCompilerBuildStep::updateEnvironment()
{
    auto bc = qobject_cast<NimBuildConfiguration *>(buildConfiguration());
    QTC_ASSERT(bc, return);
    processParameters()->setEnvironment(bc->environment());
}

void NimCompilerBuildStep::updateTargetNimFile()
{
    if (!m_targetNimFile.isEmpty())
        return;
    const Utils::FileNameList nimFiles = static_cast<NimProject *>(project())->nimFiles();
    if (!nimFiles.isEmpty())
        setTargetNimFile(nimFiles.at(0));
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
    QTest::addColumn<QList<ProjectExplorer::Task> >("tasks");
    QTest::addColumn<QString>("outputLines");

    // negative tests
    QTest::newRow("pass-through stdout")
            << "Sometext" << OutputParserTester::STDOUT
            << "Sometext\n" << QString()
            << QList<Task>()
            << QString();
    QTest::newRow("pass-through stderr")
            << "Sometext" << OutputParserTester::STDERR
            << QString() << "Sometext\n"
            << QList<Task>()
            << QString();

    // positive tests
    QTest::newRow("Parse error string")
            << QString::fromLatin1("main.nim(23, 1) Error: undeclared identifier: 'x'")
            << OutputParserTester::STDERR
            << QString("") << QString("main.nim(23, 1) Error: undeclared identifier: 'x'\n")
            << QList<Task>({Task(Task::Error,
                            "Error: undeclared identifier: 'x'",
                            Utils::FileName::fromUserInput("main.nim"), 23,
                            ProjectExplorer::Constants::TASK_CATEGORY_COMPILE)})
            << QString();

    QTest::newRow("Parse warning string")
            << QString::fromLatin1("lib/pure/parseopt.nim(56, 34) Warning: quoteIfContainsWhite is deprecated [Deprecated]")
            << OutputParserTester::STDERR
            << QString("") << QString("lib/pure/parseopt.nim(56, 34) Warning: quoteIfContainsWhite is deprecated [Deprecated]\n")
            << QList<Task>({Task(Task::Warning,
                            "Warning: quoteIfContainsWhite is deprecated [Deprecated]",
                            Utils::FileName::fromUserInput("lib/pure/parseopt.nim"), 56,
                            ProjectExplorer::Constants::TASK_CATEGORY_COMPILE)})
            << QString();
}

void NimPlugin::testNimParser()
{
    OutputParserTester testbench;
    testbench.appendOutputParser(new NimParser);
    QFETCH(QString, input);
    QFETCH(OutputParserTester::Channel, inputChannel);
    QFETCH(QList<Task>, tasks);
    QFETCH(QString, childStdOutLines);
    QFETCH(QString, childStdErrLines);
    QFETCH(QString, outputLines);

    testbench.testParsing(input, inputChannel,
                          tasks, childStdOutLines, childStdErrLines,
                          outputLines);
}

}
#endif
