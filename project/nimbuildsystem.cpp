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

#include "nimbuildsystem.h"

#include "nimproject.h"
#include "nimprojectnode.h"

#include "../nimconstants.h"

#include <projectexplorer/kitinformation.h>
#include <projectexplorer/target.h>
#include <projectexplorer/toolchain.h>

#include <utils/algorithm.h>
#include <utils/fileutils.h>
#include <utils/icon.h>
#include <utils/qtcassert.h>
#include <utils/theme/theme.h>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

using namespace ProjectExplorer;
using namespace Utils;

namespace Nim {

void NimBuildSystem::triggerParsing()
{
    QTC_ASSERT(project()->activeTarget(), return);
    QTC_ASSERT(project()->activeTarget()->kit(), return);
    Kit *kit = project()->activeTarget()->kit();
    auto tc = ToolChainKitAspect::toolChain(kit, Constants::C_NIMLANGUAGE_ID);
    QTC_ASSERT(tc, return);

    m_guard = guardParsingRun();

    m_scanner = std::make_unique<QProcess>();

    connect(m_scanner.get(), QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [this] {
        const QJsonDocument doc = QJsonDocument::fromJson(m_scanner->readAllStandardOutput());
        const QJsonValue ownPackage = Utils::filtered(doc["packages"].toArray(), [this](const QJsonValue &value) {
            return FilePath::fromString(value["manifest_path"].toString()) == project()->projectFilePath();
        })[0];

        project()->setDisplayName(ownPackage["name"].toString());

        // Collect scanned nodes
        std::vector<std::unique_ptr<FileNode>> nodes;
        for (const QJsonValue &package : doc["packages"].toArray()) {
            for (const QJsonValue &target : package["targets"].toArray()) {
                nodes.emplace_back(std::make_unique<FileNode>(FilePath::fromString(target["src_path"].toString()), FileType::Source));
            }

            nodes.emplace_back(std::make_unique<FileNode>(FilePath::fromString(package["manifest_path"].toString()), FileType::Project));
        }

        // Sync watched dirs
        const QSet<QString> fsDirs = Utils::transform<QSet>(nodes, &FileNode::directory);
        const QSet<QString> projectDirs = Utils::toSet(m_directoryWatcher.directories());
        m_directoryWatcher.addDirectories(Utils::toList(fsDirs - projectDirs), FileSystemWatcher::WatchAllChanges);
        m_directoryWatcher.removeDirectories(Utils::toList(projectDirs - fsDirs));

        // Sync project files
        const QSet<FilePath> fsFiles = Utils::transform<QSet>(nodes, &FileNode::filePath);
        const QSet<FilePath> projectFiles = Utils::toSet(project()->files([](const Node *n) { return Project::AllFiles(n); }));

        if (fsFiles != projectFiles) {
            auto projectNode = std::make_unique<ProjectNode>(project()->projectDirectory());
            projectNode->setDisplayName(project()->displayName());
            projectNode->addNestedNodes(std::move(nodes));
            project()->setRootProjectNode(std::move(projectNode));
        }

        m_guard.markAsSuccess();
        m_guard = {}; // Trigger destructor of previous object, emitting parsingFinished()

        emitBuildSystemUpdated();
    });

    const auto args = QStringList()
        << "metadata"
        << "--no-deps"
        << "--offline"
        << "--manifest-path=" + project()->projectFilePath().toString()
        << "--format-version=1";

    m_scanner->start(tc->compilerCommand().toString(), args);
}

NimBuildSystem::NimBuildSystem(Target *target)
    : BuildSystem(target)
{
    connect(&m_directoryWatcher, &FileSystemWatcher::directoryChanged, this, [this] {
        if (!isWaitingForParse())
            requestDelayedParse();
    });

    requestDelayedParse();
}

bool NimBuildSystem::supportsAction(Node *context, ProjectAction action, const Node *node) const
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

bool NimBuildSystem::addFiles(Node *, const QStringList &, QStringList *)
{
    requestDelayedParse();

    return true;
}

RemovedFilesFromProject NimBuildSystem::removeFiles(Node *,
                                                    const QStringList &,
                                                    QStringList *)
{
    requestDelayedParse();

    return RemovedFilesFromProject::Ok;
}

bool NimBuildSystem::deleteFiles(Node *, const QStringList &)
{
    return true;
}

bool NimBuildSystem::renameFile(Node *, const QString &, const QString &)
{
    requestDelayedParse();

    return true;
}

} // namespace Nim
