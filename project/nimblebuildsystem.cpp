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

#include "nimblebuildsystem.h"

#include "nimconstants.h"

#include <projectexplorer/buildconfiguration.h>
#include <projectexplorer/target.h>

#include <utils/algorithm.h>
#include <utils/hostosinfo.h>
#include <utils/qtcassert.h>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>

using namespace ProjectExplorer;
using namespace Utils;

namespace Nim {

static NimbleMetadata parseMetadata(const QString &nimblePath, const QString &projectFilePath)
{
    const auto args = QStringList()
        << "metadata"
        << "--no-deps"
        << "--offline"
        << "--manifest-path=" + projectFilePath
        << "--format-version=1";

    QProcess process;
    process.start(nimblePath, args);
    process.waitForFinished();

    const QJsonDocument doc = QJsonDocument::fromJson(process.readAllStandardOutput());
    const QJsonValue ownPackage = Utils::filtered(doc["packages"].toArray(), [projectFilePath](const QJsonValue &value) {
        return value["manifest_path"].toString() == projectFilePath;
    })[0];

    NimbleMetadata result = {};

    result.projectName = ownPackage["name"].toString();

    for (const QJsonValue &target : ownPackage["targets"].toArray()) {
        const QString kind = target["kind"][0].toString();
        const QString name = target["name"].toString();
        if (kind == "bin") {
            result.bin.append(name);
        }
    }

    return result;
}

NimbleBuildSystem::NimbleBuildSystem(Target *target)
    : BuildSystem(target), m_projectScanner(target->project())
{
    m_projectScanner.watchProjectFilePath();

    connect(&m_projectScanner, &NimProjectScanner::fileChanged, this, [this](const QString &path) {
        if (path == projectFilePath().toString())
            requestDelayedParse();
    });

    connect(&m_projectScanner, &NimProjectScanner::requestReparse,
            this, &NimbleBuildSystem::requestDelayedParse);

    connect(&m_projectScanner, &NimProjectScanner::finished, this, &NimbleBuildSystem::updateProject);

    connect(&m_projectScanner, &NimProjectScanner::directoryChanged, this, [this] (const QString &directory){
        // Workaround for nimble creating temporary files in project root directory
        // when querying the list of tasks.
        // See https://github.com/nim-lang/nimble/issues/720
        if (directory != projectDirectory().toString())
            requestDelayedParse();
    });

    requestDelayedParse();
}

void NimbleBuildSystem::triggerParsing()
{
    // Only allow one parsing run at the same time:
    auto guard = guardParsingRun();
    if (!guard.guardsProject())
        return;
    m_guard = std::move(guard);

    m_projectScanner.startScan();
}

void NimbleBuildSystem::updateProject()
{
    const FilePath projectDir = projectDirectory();
    const FilePath nimble = Nim::nimblePathFromKit(kit());

    const NimbleMetadata metadata = parseMetadata(nimble.toString(), projectDir.toString());

    QList<BuildTargetInfo> targets;
    std::vector<NimbleTask> tasks;

    if (target()->activeBuildConfiguration()) {
        const FilePath buildDirectory = target()->activeBuildConfiguration()->buildDirectory();

        targets = Utils::transform(metadata.bin, [&](const QString &bin) {
            BuildTargetInfo info = {};
            info.displayName = bin;
            info.targetFilePath = buildDirectory.pathAppended(HostOsInfo::withExecutableSuffix(bin));
            info.projectFilePath = projectFilePath();
            info.workingDirectory = buildDirectory;
            info.buildKey = bin;
            return info;
        });

        tasks = Utils::transform(metadata.bin, [&](const QString &bin) {
            NimbleTask task = {};
            task.name = bin;
            task.description = bin;
            return task;
        }).toVector().toStdVector();
    }

    setApplicationTargets(std::move(targets));
    project()->setDisplayName(metadata.projectName);

    if (tasks != m_tasks) {
        m_tasks = std::move(tasks);
        emit tasksChanged();
    }

    // Complete scan
    m_guard.markAsSuccess();
    m_guard = {};

    emitBuildSystemUpdated();
}

std::vector<NimbleTask> NimbleBuildSystem::tasks() const
{
    return m_tasks;
}

bool NimbleBuildSystem::supportsAction(Node *context, ProjectAction action, const Node *node) const
{
    if (node->asFileNode()) {
        return action == ProjectAction::Rename
            || action == ProjectAction::RemoveFile;
    }
    if (node->isFolderNodeType() || node->isProjectNodeType()) {
        return action == ProjectAction::AddNewFile
            || action == ProjectAction::RemoveFile
            || action == ProjectAction::AddExistingFile;
    }
    return BuildSystem::supportsAction(context, action, node);
}

bool NimbleBuildSystem::addFiles(Node *, const QStringList &filePaths, QStringList *)
{
    return m_projectScanner.addFiles(filePaths);
}

RemovedFilesFromProject NimbleBuildSystem::removeFiles(Node *,
                                                       const QStringList &filePaths,
                                                       QStringList *)
{
    return m_projectScanner.removeFiles(filePaths);
}

bool NimbleBuildSystem::deleteFiles(Node *, const QStringList &)
{
    return true;
}

bool NimbleBuildSystem::renameFile(Node *, const QString &filePath, const QString &newFilePath)
{
    return m_projectScanner.renameFile(filePath, newFilePath);
}

} // Nim
