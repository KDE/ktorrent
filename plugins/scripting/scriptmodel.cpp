/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QIcon>
#include <QMimeDatabase>
#include <QMimeType>

#include <KLocalizedString>
#include <KTar>
#include <KZip>

#include <interfaces/functions.h>
#include <util/error.h>
#include <util/fileops.h>
#include <util/log.h>

#include "script.h"
#include "scriptmodel.h"

using namespace bt;

namespace kt
{
ScriptModel::ScriptModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

ScriptModel::~ScriptModel()
{
}

void ScriptModel::addScript(const QString &file)
{
    Out(SYS_SCR | LOG_NOTICE) << "Adding script from " << file << endl;
    QMimeDatabase db;
    QMimeType ptr = db.mimeTypeForFile(file);

    bool is_tar = ptr.name() == QStringLiteral("application/x-compressed-tar") || ptr.name() == QStringLiteral("application/x-bzip-compressed-tar");
    bool is_zip = ptr.name() == QStringLiteral("application/zip");
    if (is_tar || is_zip) {
        // It's a package
        if (is_tar) {
            KTar tar(file);
            addScriptFromArchive(&tar);
        } else {
            KZip zip(file);
            addScriptFromArchive(&zip);
        }
    } else {
        // make sure we don't add dupes
        for (Script *s : qAsConst(scripts))
            if (s->scriptFile() == file)
                return;

        Script *s = new Script(file, this);
        scripts.append(s);
        insertRow(scripts.count() - 1);
    }
}

Script *ScriptModel::addScriptFromDesktopFile(const QString &dir, const QString &desktop_file)
{
    Script *s = new Script(this);
    if (!s->loadFromDesktopFile(dir, desktop_file)) {
        delete s;
        return 0;
    }

    // we don't want dupes
    for (Script *os : qAsConst(scripts)) {
        if (s->scriptFile() == os->scriptFile()) {
            delete s;
            return 0;
        }
    }

    s->setPackageDirectory(dir);
    scripts.append(s);
    insertRow(scripts.count() - 1);
    return s;
}

void ScriptModel::addScriptFromArchive(KArchive *archive)
{
    if (!archive->open(QIODevice::ReadOnly))
        throw bt::Error(i18n("Cannot open archive for reading."));

    const KArchiveDirectory *dir = archive->directory();
    if (!dir)
        throw bt::Error(i18n("Invalid archive."));

    const QStringList entries = dir->entries();
    for (const QString &e : entries) {
        const KArchiveEntry *entry = dir->entry(e);
        if (entry && entry->isDirectory()) {
            addScriptFromArchiveDirectory((const KArchiveDirectory *)entry);
        }
    }
}

void ScriptModel::addScriptFromArchiveDirectory(const KArchiveDirectory *dir)
{
    const QStringList files = dir->entries();
    for (const QString &file : files) {
        // look for the desktop file
        if (!file.endsWith(QStringLiteral(".desktop")) && !file.endsWith(QStringLiteral(".DESKTOP")))
            continue;

        // check for duplicate packages
        QString dest_dir = kt::DataDir() + QStringLiteral("scripts/") + dir->name() + QLatin1Char('/');
        for (Script *s : qAsConst(scripts)) {
            if (s->packageDirectory() == dest_dir)
                throw bt::Error(i18n("There is already a script package named %1 installed.", dir->name()));
        }

        // extract to the scripts dir
        dir->copyTo(dest_dir, true);
        if (!addScriptFromDesktopFile(dest_dir, file))
            throw bt::Error(i18n("Failed to load script from archive. There is something wrong with the desktop file."));

        return;
    }

    throw bt::Error(i18n("No script found in archive."));
}

int ScriptModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : scripts.count();
}

QVariant ScriptModel::data(const QModelIndex &index, int role) const
{
    Script *s = scriptForIndex(index);
    if (!s)
        return QVariant();

    switch (role) {
    case Qt::DisplayRole:
        return s->name();
    case Qt::DecorationRole:
        return s->iconName();
    case Qt::CheckStateRole:
        return s->running();
    case Qt::ToolTipRole:
        if (s->executeable())
            return i18n("<b>%1</b><br/><br/>%2", s->name(), s->metaInfo().comment);
        else
            return i18n(
                "No interpreter for this script could be found, so it cannot be executed. "
                "Please make sure the right interpreter is installed.<br/><br/>"
                "<b>Hint:</b> All standard ktorrent scripts require krosspython");
    case CommentRole:
        return s->metaInfo().comment;
    case ConfigurableRole:
        return s->running() && s->hasConfigure();
    default:
        return QVariant();
    }
}

bool ScriptModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;

    Script *s = scriptForIndex(index);
    if (!s)
        return false;

    if (role == Qt::CheckStateRole) {
        if (value.toBool())
            s->execute();
        else
            s->stop();

        dataChanged(index, index);
        return true;
    } else if (role == ConfigureRole) {
        s->configure();
        return true;
    } else if (role == AboutRole) {
        showPropertiesDialog(s);
        return true;
    }
    return false;
}

Qt::ItemFlags ScriptModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return QAbstractItemModel::flags(index);

    Script *s = scriptForIndex(index);
    if (!s)
        return QAbstractItemModel::flags(index);

    if (s->executeable())
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
    else
        return Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
}

Script *ScriptModel::scriptForIndex(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    if (index.row() < 0 || index.row() >= scripts.count())
        return 0;

    return scripts[index.row()];
}

bool ScriptModel::removeRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent);
    beginRemoveRows(QModelIndex(), row, row + count - 1);
    endRemoveRows();
    return true;
}

bool ScriptModel::insertRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent);
    beginInsertRows(QModelIndex(), row, row + count - 1);
    endInsertRows();
    return true;
}

QStringList ScriptModel::scriptFiles() const
{
    QStringList ret;
    for (Script *s : qAsConst(scripts))
        ret << s->scriptFile();
    return ret;
}

QStringList ScriptModel::runningScriptFiles() const
{
    QStringList ret;
    for (Script *s : qAsConst(scripts)) {
        if (s->running())
            ret << s->scriptFile();
    }
    return ret;
}

void ScriptModel::runScripts(const QStringList &r)
{
    int idx = 0;
    for (Script *s : qAsConst(scripts)) {
        if (r.contains(s->scriptFile()) && !s->running()) {
            s->execute();
            QModelIndex i = index(idx, 0);
            Q_EMIT dataChanged(i, i);
        }
        idx++;
    }
}

void ScriptModel::removeScripts(const QModelIndexList &indices)
{
    QList<Script *> to_remove;

    for (const QModelIndex &idx : indices) {
        Script *s = scriptForIndex(idx);
        if (s && s->removable())
            to_remove << s;
    }

    beginResetModel();
    for (Script *s : qAsConst(to_remove)) {
        if (!s->packageDirectory().isEmpty())
            bt::Delete(s->packageDirectory(), true);
        scripts.removeAll(s);
        s->stop();
        s->deleteLater();
    }

    endResetModel();
}
}
