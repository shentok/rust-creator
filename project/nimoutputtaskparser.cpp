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

#include "nimoutputtaskparser.h"

#include <QDesktopServices>
#include <QRegularExpression>

#include <algorithm>

using namespace ProjectExplorer;
using namespace Utils;

namespace Nim {

NimParser::Result NimParser::handleLine(const QString &line, Utils::OutputFormat)
{
    static const QRegularExpression error(QStringLiteral("^error(\\[E\\d+\\])?: "));
    static const QRegularExpression warning(QStringLiteral("^warning(\\[.\\d+\\])?: "));
    static const QRegularExpression location(QStringLiteral("^\\s*--> (.*):(\\d+):(\\d+)"));
    static const QRegularExpression https(QStringLiteral("https?\\://[^\\s]+"));

    if (error.match(line).hasMatch()) {
        m_type = Task::Error;
        m_message.append(line);
        return Status::InProgress;
    }

    if (warning.match(line).hasMatch()) {
        m_type = Task::Warning;
        m_message.append(line);
        return Status::InProgress;
    }

    const QRegularExpressionMatch locationMatch = location.match(line);
    if (locationMatch.hasMatch()) {
        LinkSpecs linkSpecs;
        const FilePath filePath = absoluteFilePath(FilePath::fromUserInput(locationMatch.captured(1)));
        const int lineNumber = locationMatch.captured(2).toInt();
//        const int rowNumber = locationMatch.captured(3).toInt();
        addLinkSpecForAbsoluteFilePath(linkSpecs, filePath, lineNumber, locationMatch, 1);
        m_linkSpecs << adjustedLinkSpecs(linkSpecs, m_message);
        m_message.append(line);
        if (m_fileName.isEmpty())
        {
            m_fileName = filePath;
            m_lineNumber = lineNumber;
        }
        return {Status::InProgress, linkSpecs};
    }

    if (m_message.isEmpty()) {
        return Status::NotHandled;
    }

    const QRegularExpressionMatch httpsMatch = https.match(line);
    if (httpsMatch.hasMatch())
    {
        LinkSpecs linkSpecs;
        linkSpecs << LinkSpec(httpsMatch.capturedStart(0), httpsMatch.capturedLength(0), httpsMatch.captured());
        m_linkSpecs << adjustedLinkSpecs(linkSpecs, m_message);
        m_message.append(line);
        return {Status::InProgress, linkSpecs};
    }

    if (!line.trimmed().isEmpty()) {
        m_message.append(line);
        return Status::InProgress;
    }

    CompileTask t(m_type, m_message.join('\n'), m_fileName, m_lineNumber);
    setDetailsFormat(t, m_linkSpecs);
    scheduleTask(t, m_message.size());

    m_message.clear();
    m_fileName.clear();
    m_lineNumber = 0;
    m_linkSpecs.clear();
    m_type = Task::Unknown;

    return Status::Done;
}

bool NimParser::handleLink(const QString &href)
{
    static const QRegularExpression https(QStringLiteral("https?\\://[^\\s]+"));

    if (https.match(href).hasMatch())
    {
        QDesktopServices::openUrl(QUrl(href));
        return true;
    }

    return false;
}

OutputLineParser::LinkSpecs NimParser::adjustedLinkSpecs(const LinkSpecs &original, const QStringList &message)
{
    const QString details = CompileTask(Task::Unknown, message.join("\n")).details.join("\n");
    const int offset = details.isEmpty() ? 0 : (details.size() + 1);
    LinkSpecs adjustedLinkSpecs = original;
    for (LinkSpec &ls : adjustedLinkSpecs)
        ls.startPos += offset;
    return adjustedLinkSpecs;
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

    auto compileTask = [](Task::TaskType type,
            const QString &description,
            const Utils::FilePath &file,
            int line,
            const QVector<QTextLayout::FormatRange> &formats)
    {
        CompileTask task(type, description, file, line);
        task.formats = formats;
        return task;
    };

    auto formatRange = [](int start, int length, const QString &anchorHref = QString())
    {
        QTextCharFormat format;
        format.setAnchorHref(anchorHref);

        return QTextLayout::FormatRange{start, length, format};
    };

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
    QTest::newRow("Parse error message")
            << "error[E0283]: type annotations needed\n"
               "   --> src/cargo/util/config/de.rs:530:63\n"
               "    |\n"
               "530 |                 seed.deserialize(Tuple2Deserializer(1i32, env.as_ref()))\n"
               "    |                                                           ----^^^^^^--\n"
               "    |                                                           |   |\n"
               "    |                                                           |   cannot infer type for type parameter `T` declared on the trait `AsRef`\n"
               "    |                                                           this method call resolves to `&T`\n"
               "    |\n"
               "    = note: cannot satisfy `std::string::String: std::convert::AsRef<_>`\n"
               "\n"
               "error: aborting due to previous error\n"
               "\n"
               "For more information about this error, try `rustc --explain E0283`.\n"
            << OutputParserTester::STDERR
            << QString() << QString("For more information about this error, try `rustc --explain E0283`.\n\n")
            << (Tasks()
                << compileTask(Task::Error,
                               "error[E0283]: type annotations needed\n"
                               "   --> src/cargo/util/config/de.rs:530:63\n"
                               "    |\n"
                               "530 |                 seed.deserialize(Tuple2Deserializer(1i32, env.as_ref()))\n"
                               "    |                                                           ----^^^^^^--\n"
                               "    |                                                           |   |\n"
                               "    |                                                           |   cannot infer type for type parameter `T` declared on the trait `AsRef`\n"
                               "    |                                                           this method call resolves to `&T`\n"
                               "    |\n"
                               "    = note: cannot satisfy `std::string::String: std::convert::AsRef<_>`",
                               FilePath::fromUserInput("/usr/src/src/cargo/util/config/de.rs"),
                               530,
                               QVector<QTextLayout::FormatRange>()
                                   << formatRange(38, 7)
                                   << formatRange(45, 27, "olpfile:///usr/src/src/cargo/util/config/de.rs::530::-1")
                                   << formatRange(72, 555))
                << CompileTask(Task::Error,
                               "error: aborting due to previous error"))
            << QString();

    QTest::newRow("Parse warning string")
            << "warning: manual `RangeInclusive::contains` implementation\n"
               "   --> crates/cargo-platform/src/cfg.rs:301:18\n"
               "    |\n"
               "301 |     ch == '_' || ('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z')\n"
               "    |                  ^^^^^^^^^^^^^^^^^^^^^^^^ help: use: `('a'..='z').contains(&ch)`\n"
               "    |\n"
               "    = note: `#[warn(clippy::manual_range_contains)]` on by default\n"
               "    = help: for further information visit https://rust-lang.github.io/rust-clippy/master/index.html#manual_range_contains\n"
               "\n"
            << OutputParserTester::STDERR
            << QString()
            << QString("\n")
            << Tasks({compileTask(Task::Warning,
                      "warning: manual `RangeInclusive::contains` implementation\n"
                      "   --> crates/cargo-platform/src/cfg.rs:301:18\n"
                      "    |\n"
                      "301 |     ch == '_' || ('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z')\n"
                      "    |                  ^^^^^^^^^^^^^^^^^^^^^^^^ help: use: `('a'..='z').contains(&ch)`\n"
                      "    |\n"
                      "    = note: `#[warn(clippy::manual_range_contains)]` on by default\n"
                      "    = help: for further information visit https://rust-lang.github.io/rust-clippy/master/index.html#manual_range_contains",
                      FilePath::fromUserInput("/usr/src/crates/cargo-platform/src/cfg.rs"),
                      301,
                      QVector<QTextLayout::FormatRange>()
                          << formatRange(58, 7)
                          << formatRange(65, 32, "olpfile:///usr/src/crates/cargo-platform/src/cfg.rs::301::-1")
                          << formatRange(97, 292)
                          << formatRange(389, 79, "https://rust-lang.github.io/rust-clippy/master/index.html#manual_range_contains"))})
            << QString();

    QTest::newRow("Parse warning string 2")
            << "warning: stripping a prefix manually\n"
               "   --> src/cargo/core/compiler/custom_build.rs:353:56\n"
               "    |\n"
               "353 |                         warnings_in_case_of_panic.push(stdout[CARGO_WARNING.len()..].to_owned());\n"
               "    |                                                        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n"
               "    |\n"
               "    = note: `#[warn(clippy::manual_strip)]` on by default\n"
               "note: the prefix was tested here\n"
               "   --> src/cargo/core/compiler/custom_build.rs:352:21\n"
               "    |\n"
               "352 |                     if stdout.starts_with(CARGO_WARNING) {\n"
               "    |                     ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n"
               "    = help: for further information visit https://rust-lang.github.io/rust-clippy/master/index.html#manual_strip\n"
               "help: try using the `strip_prefix` method\n"
               "    |\n"
               "352 |                     if let Some(<stripped>) = stdout.strip_prefix(CARGO_WARNING) {\n"
               "353 |                         warnings_in_case_of_panic.push(<stripped>.to_owned());\n"
               "    |\n"
            << OutputParserTester::STDERR
            << QString()
            << QString()
            << Tasks({compileTask(Task::Warning,
                      "warning: stripping a prefix manually\n"
                      "   --> src/cargo/core/compiler/custom_build.rs:353:56\n"
                      "    |\n"
                      "353 |                         warnings_in_case_of_panic.push(stdout[CARGO_WARNING.len()..].to_owned());\n"
                      "    |                                                        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n"
                      "    |\n"
                      "    = note: `#[warn(clippy::manual_strip)]` on by default\n"
                      "note: the prefix was tested here\n"
                      "   --> src/cargo/core/compiler/custom_build.rs:352:21\n"
                      "    |\n"
                      "352 |                     if stdout.starts_with(CARGO_WARNING) {\n"
                      "    |                     ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n"
                      "    = help: for further information visit https://rust-lang.github.io/rust-clippy/master/index.html#manual_strip\n"
                      "help: try using the `strip_prefix` method\n"
                      "    |\n"
                      "352 |                     if let Some(<stripped>) = stdout.strip_prefix(CARGO_WARNING) {\n"
                      "353 |                         warnings_in_case_of_panic.push(<stripped>.to_owned());\n"
                      "    |",
                      FilePath::fromUserInput("/usr/src/src/cargo/core/compiler/custom_build.rs"),
                      353,
                      QVector<QTextLayout::FormatRange>()
                          << formatRange(37, 7)
                          << formatRange(44, 39, "olpfile:///usr/src/src/cargo/core/compiler/custom_build.rs::353::-1")
                          << formatRange(83, 313)
                          << formatRange(396, 39, "olpfile:///usr/src/src/cargo/core/compiler/custom_build.rs::352::-1")
                          << formatRange(435, 185)
                          << formatRange(620, 70, "https://rust-lang.github.io/rust-clippy/master/index.html#manual_strip")
                          << formatRange(690, 228))})
            << QString();
}

void NimPlugin::testNimParser()
{
    auto parser = new NimParser;
    parser->addSearchDir(Utils::FilePath::fromString("/usr/src"));
    parser->skipFileExistsCheck();
    OutputParserTester testbench;
    testbench.addLineParser(parser);
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
