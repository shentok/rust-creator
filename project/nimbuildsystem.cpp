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

const char SETTINGS_KEY[] = "Rust.BuildSystem";
const char EXCLUDED_FILES_KEY[] = "ExcludedFiles";

RustCWorker::RustCWorker(ProjectNode* node, QObject* parent) :
    QObject(parent),
    m_rustc(this),
    m_node(node)
{
    m_fileSaver.setAutoRemove(true);
}

void RustCWorker::start(const FilePath &mainSourceFile)
{
    m_rustc.setWorkingDirectory(mainSourceFile.toFileInfo().dir().absolutePath());

    connect(&m_rustc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [this] {
        if (!m_node)
            return;

        std::vector<std::unique_ptr<FileNode>> files;
        QTextStream ts(m_fileSaver.file());
        while (!ts.atEnd())
        {
            QString line = ts.readLine().trimmed();
            line.chop(1);
            auto file = FilePath::fromString(m_rustc.workingDirectory()).pathAppended(line);
            qDebug() << file;
            auto mainSourceFile = std::make_unique<FileNode>(file, FileType::Source);
            files.emplace_back(std::move(mainSourceFile));
        }

        m_node->addNestedNodes(std::move(files));

        deleteLater();
    });


    const QStringList args = QStringList()
            << "--emit"
            << "dep-info"
            << "-o"
            << m_fileSaver.fileName()
            << mainSourceFile.toString();

    m_rustc.start("rustc", args);
}

NimProjectScanner::NimProjectScanner(Project *project)
    : m_project(project)
{
    connect(&m_directoryWatcher, &FileSystemWatcher::directoryChanged,
            this, &NimProjectScanner::directoryChanged);
    connect(&m_directoryWatcher, &FileSystemWatcher::fileChanged,
            this, &NimProjectScanner::fileChanged);

    connect(m_project, &Project::settingsLoaded, this, &NimProjectScanner::loadSettings);
    connect(m_project, &Project::aboutToSaveSettings, this, &NimProjectScanner::saveSettings);

    connect(&m_scanner, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [this] {
        const QJsonDocument doc = QJsonDocument::fromJson(m_scanner.readAllStandardOutput());
        const QJsonValue ownPackage = Utils::filtered(doc["packages"].toArray(), [this](const QJsonValue &value) {
            return FilePath::fromString(value["manifest_path"].toString()) == m_project->projectFilePath();
        })[0];

        m_project->setDisplayName(ownPackage["name"].toString());

        auto projectNode = std::make_unique<ProjectNode>(m_project->projectDirectory());
        projectNode->setDisplayName(m_project->displayName());
        projectNode->setIcon(QIcon(":/rust/images/ferris.png"));

        // Collect scanned nodes
        for (const QJsonValue &package : doc["packages"].toArray()) {
            auto subProjectNode = std::make_unique<ProjectNode>(FilePath::fromString(package["manifest_path"].toString()));
            subProjectNode->setDisplayName(package["name"].toString());
            subProjectNode->setIcon(QIcon(":/rust/images/package.png"));

            for (const QJsonValue &target : package["targets"].toArray()) {
                auto targetNode = std::make_unique<ProjectNode>(FilePath::fromString(package["manifest_path"].toString()));
                targetNode->setDisplayName(target["name"].toString());
                const QString iconPath = ":/rust/images/" + target["kind"].toArray()[0].toString() + ".png";
                targetNode->setIcon(QIcon(QFileInfo::exists(iconPath) ? iconPath : ":/rust/images/target.png"));
                RustCWorker *worker = new RustCWorker(targetNode.get(), this);
                subProjectNode->addNode(std::move(targetNode));

                worker->start(FilePath::fromString(target["src_path"].toString()));
            }

            auto cargoTomlNode = std::make_unique<FileNode>(FilePath::fromString(package["manifest_path"].toString()), FileType::Project);
            subProjectNode->addNode(std::move(cargoTomlNode));

            projectNode->addNode(std::move(subProjectNode));
        }

        // Sync watched dirs
        const QSet<QString> fsDirs = Utils::transform<QSet>(projectNode->nodes(), &FileNode::directory);
        const QSet<QString> projectDirs = Utils::toSet(m_directoryWatcher.directories());
        m_directoryWatcher.addDirectories(Utils::toList(fsDirs - projectDirs), FileSystemWatcher::WatchAllChanges);
        m_directoryWatcher.removeDirectories(Utils::toList(projectDirs - fsDirs));

        // Sync project files
        const QSet<FilePath> fsFiles = Utils::transform<QSet>(projectNode->nodes(), &FileNode::filePath);
        const QSet<FilePath> projectFiles = Utils::toSet(m_project->files([](const Node *n) { return Project::AllFiles(n); }));

//        if (fsFiles != projectFiles)
            m_project->setRootProjectNode(std::move(projectNode));

        emit finished();
    });
}

void NimProjectScanner::loadSettings()
{
    QVariantMap settings = m_project->namedSettings(SETTINGS_KEY).toMap();
    if (settings.contains(EXCLUDED_FILES_KEY))
        setExcludedFiles(settings.value(EXCLUDED_FILES_KEY, excludedFiles()).toStringList());

    emit requestReparse();
}

void NimProjectScanner::saveSettings()
{
    QVariantMap settings;
    settings.insert(EXCLUDED_FILES_KEY, excludedFiles());
    m_project->setNamedSettings(SETTINGS_KEY, settings);
}

void NimProjectScanner::startScan()
{
    QTC_ASSERT(m_project->activeTarget(), return);
    QTC_ASSERT(m_project->activeTarget()->kit(), return);
    Kit *kit = m_project->activeTarget()->kit();
    auto tc = ToolChainKitAspect::toolChain(kit, Constants::C_NIMLANGUAGE_ID);
    QTC_ASSERT(tc, return);

    const auto args = QStringList()
        << "metadata"
        << "--no-deps"
        << "--offline"
        << "--manifest-path=" + m_project->projectFilePath().toString()
        << "--format-version=1";

    m_scanner.start(tc->compilerCommand().toString(), args);
}

void NimProjectScanner::watchProjectFilePath()
{
    m_directoryWatcher.addFile(m_project->projectFilePath().toString(), FileSystemWatcher::WatchModifiedDate);
}

void NimProjectScanner::setExcludedFiles(const QStringList &list)
{
    static_cast<NimProject *>(m_project)->setExcludedFiles(list);
}

QStringList NimProjectScanner::excludedFiles() const
{
    return static_cast<NimProject *>(m_project)->excludedFiles();
}

bool NimProjectScanner::addFiles(const QStringList &filePaths)
{
    setExcludedFiles(Utils::filtered(excludedFiles(), [&](const QString & f) {
        return !filePaths.contains(f);
    }));

    requestReparse();

    return true;
}

RemovedFilesFromProject NimProjectScanner::removeFiles(const QStringList &filePaths)
{
    setExcludedFiles(Utils::filteredUnique(excludedFiles() + filePaths));

    requestReparse();

    return RemovedFilesFromProject::Ok;
}

bool NimProjectScanner::renameFile(const QString &, const QString &to)
{
    QStringList files = excludedFiles();
    files.removeOne(to);
    setExcludedFiles(files);

    requestReparse();

    return true;
}

NimBuildSystem::NimBuildSystem(Target *target)
    : BuildSystem(target), m_projectScanner(target->project())
{
    connect(&m_projectScanner, &NimProjectScanner::finished, this, [this] {
        m_guard.markAsSuccess();
        m_guard = {}; // Trigger destructor of previous object, emitting parsingFinished()

        emitBuildSystemUpdated();
    });

    connect(&m_projectScanner, &NimProjectScanner::requestReparse,
            this, &NimBuildSystem::requestDelayedParse);

    connect(&m_projectScanner, &NimProjectScanner::directoryChanged, this, [this] {
        if (!isWaitingForParse())
            requestDelayedParse();
    });

    requestDelayedParse();
}

void NimBuildSystem::triggerParsing()
{
    m_guard = guardParsingRun();
    m_projectScanner.startScan();
}

void NimBuildSystem::loadSettings()
{
    QVariantMap settings = project()->namedSettings(SETTINGS_KEY).toMap();
    if (settings.contains(EXCLUDED_FILES_KEY))
        m_projectScanner.setExcludedFiles(settings.value(EXCLUDED_FILES_KEY, m_projectScanner.excludedFiles()).toStringList());

    requestParse();
}

void NimBuildSystem::saveSettings()
{
    QVariantMap settings;
    settings.insert(EXCLUDED_FILES_KEY, m_projectScanner.excludedFiles());
    project()->setNamedSettings(SETTINGS_KEY, settings);
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

bool NimBuildSystem::addFiles(Node *, const QStringList &filePaths, QStringList *)
{
    return m_projectScanner.addFiles(filePaths);
}

RemovedFilesFromProject NimBuildSystem::removeFiles(Node *,
                                                    const QStringList &filePaths,
                                                    QStringList *)
{
    return m_projectScanner.removeFiles(filePaths);
}

bool NimBuildSystem::deleteFiles(Node *, const QStringList &)
{
    return true;
}

bool NimBuildSystem::renameFile(Node *, const QString &filePath, const QString &newFilePath)
{
    return m_projectScanner.renameFile(filePath, newFilePath);
}

} // namespace Nim
