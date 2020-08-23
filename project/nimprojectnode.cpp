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

#include "nimprojectnode.h"

#include "nimbuildsystem.h"

#include <projectexplorer/project.h>
#include <projectexplorer/target.h>

#include <utils/algorithm.h>

using namespace ProjectExplorer;

namespace Nim {

NimProjectNode::NimProjectNode(const Utils::FilePath &projectFilePath)
    : ProjectNode(projectFilePath)
{}

NimTargetNode::NimTargetNode(const BuildTargetInfo &buildTargetInfo) :
    ProjectExplorer::ProjectNode(buildTargetInfo.projectFilePath),
    m_targetBuildInfo(buildTargetInfo)
{
    setPriority(Node::DefaultProjectPriority + 900);
    setIcon(QIcon(":/projectexplorer/images/build.png")); // TODO: Use proper icon!
    setListInProject(false);
    setProductType(ProductType::Other);
}

QString NimTargetNode::tooltip() const
{
    return m_tooltip;
}

QString NimTargetNode::buildKey() const
{
    return m_targetBuildInfo.buildKey;
}

Utils::optional<Utils::FilePath> NimTargetNode::visibleAfterAddFileAction() const
{
    return filePath().pathAppended("CMakeLists.txt");
}

void NimTargetNode::setTargetInformation(const QList<Utils::FilePath> &artifacts,
                                           const QString &type)
{
    m_tooltip = QCoreApplication::translate("CMakeTargetNode", "Target type: ") + type + "<br>";
    if (artifacts.isEmpty()) {
        m_tooltip += QCoreApplication::translate("CMakeTargetNode", "No build artifacts");
    } else {
        const QStringList tmp = Utils::transform(artifacts, &Utils::FilePath::toUserOutput);
        m_tooltip += QCoreApplication::translate("CMakeTargetNode", "Build artifacts:") + "<br>"
                + tmp.join("<br>");
    }
}

} // namespace Nim
