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

#include "nimprojectnode.h"

#include <projectexplorer/project.h>
#include <projectexplorer/target.h>

#include <utils/algorithm.h>
#include <utils/fileutils.h>
#include <utils/qtcassert.h>

using namespace ProjectExplorer;
using namespace Utils;

namespace Nim {

NimProjectScanner::NimProjectScanner(Project *project)
    : m_project(project)
{
    connect(&m_directoryWatcher, &FileSystemWatcher::directoryChanged,
            this, &NimProjectScanner::directoryChanged);
    connect(&m_directoryWatcher, &FileSystemWatcher::fileChanged,
            this, &NimProjectScanner::fileChanged);
}

void NimProjectScanner::startScan()
{
    m_scanner = std::make_unique<TreeScanner>();
    m_scanner->setFilter([](const Utils::MimeType &, const FilePath &fp) {
        const QString path = fp.toString();
        return path.endsWith(".nimproject")
                || path.contains(".nimproject.user")
                || path.contains(".nimble.user");
    });

    connect(m_scanner.get(), &TreeScanner::finished, this, [this] {
        // Collect scanned nodes
        std::vector<std::unique_ptr<FileNode>> nodes;
        for (FileNode *node : m_scanner->release()) {
            if (!node->path().endsWith(".nim") && !node->path().endsWith(".nimble"))
                node->setEnabled(false); // Disable files that do not end in .nim
            nodes.emplace_back(node);
        }

        // Sync watched dirs
        const QSet<QString> fsDirs = Utils::transform<QSet>(nodes, &FileNode::directory);
        const QSet<QString> projectDirs = Utils::toSet(m_directoryWatcher.directories());
        m_directoryWatcher.addDirectories(Utils::toList(fsDirs - projectDirs), FileSystemWatcher::WatchAllChanges);
        m_directoryWatcher.removeDirectories(Utils::toList(projectDirs - fsDirs));

        // Sync project files
        const QSet<FilePath> fsFiles = Utils::transform<QSet>(nodes, &FileNode::filePath);
        const QSet<FilePath> projectFiles = Utils::toSet(m_project->files([](const Node *n) { return Project::AllFiles(n); }));

        if (fsFiles != projectFiles) {
            auto projectNode = std::make_unique<ProjectNode>(m_project->projectDirectory());
            projectNode->setDisplayName(m_project->displayName());
            projectNode->addNestedNodes(std::move(nodes));
            m_project->setRootProjectNode(std::move(projectNode));
        }

        emit finished();
    });

    m_scanner->asyncScanForFiles(m_project->projectDirectory());
}

void NimProjectScanner::watchProjectFilePath()
{
    m_directoryWatcher.addFile(m_project->projectFilePath().toString(), FileSystemWatcher::WatchModifiedDate);
}

bool NimProjectScanner::addFiles(const QStringList &)
{
    requestReparse();

    return true;
}

RemovedFilesFromProject NimProjectScanner::removeFiles(const QStringList &)
{
    requestReparse();

    return RemovedFilesFromProject::Ok;
}

bool NimProjectScanner::renameFile(const QString &, const QString &)
{
    requestReparse();

    return true;
}

} // namespace Nim
