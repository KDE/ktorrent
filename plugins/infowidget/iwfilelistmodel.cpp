/*
    SPDX-FileCopyrightText: 2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2007 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "iwfilelistmodel.h"

#include <cmath>

#include <KLocalizedString>

#include <QFileInfo>

#include "infowidgetpluginsettings.h"
#include <interfaces/functions.h>
#include <interfaces/torrentfileinterface.h>
#include <interfaces/torrentinterface.h>
#include <torrent/torrentcontrol.h>
#include <util/functions.h>

using namespace bt;

namespace kt
{
IWFileListModel::IWFileListModel(bt::TorrentInterface *tc, QObject *parent)
    : TorrentFileListModel(tc, KEEP_FILES, parent)
{
    mmfile = tc ? IsMultimediaFile(tc->getStats().output_path) : 0;
    preview = false;
    percentage = 0;
}

IWFileListModel::~IWFileListModel()
{
}

void IWFileListModel::changeTorrent(bt::TorrentInterface *tc)
{
    kt::TorrentFileListModel::changeTorrent(tc);
    mmfile = tc ? IsMultimediaFile(tc->getStats().output_path) : 0;
    preview = false;
    percentage = 0;
}

int IWFileListModel::columnCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return 5;
    else
        return 0;
}

QVariant IWFileListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
        return QVariant();

    if (section < 2)
        return TorrentFileListModel::headerData(section, orientation, role);

    switch (section) {
    case 2:
        return i18n("Priority");
    case 3:
        return i18nc("@title:column", "Preview");
    // xgettext: no-c-format
    case 4:
        return i18nc("Percent of File Downloaded", "% Complete");
    default:
        return QVariant();
    }
}

static QString PriorityString(const bt::TorrentFileInterface *file)
{
    switch (file->getPriority()) {
    case FIRST_PREVIEW_PRIORITY:
    case FIRST_PRIORITY:
        return i18nc("Download first", "First");
    case LAST_PREVIEW_PRIORITY:
    case LAST_PRIORITY:
        return i18nc("Download last", "Last");
    case ONLY_SEED_PRIORITY:
    case EXCLUDED:
        return QString();
    case NORMAL_PREVIEW_PRIORITY:
    default:
        return i18nc("Download Normal (not as first or last)", "Normal");
    }
}

QVariant IWFileListModel::data(const QModelIndex &index, int role) const
{
    if (index.column() < 2 && role != Qt::ForegroundRole)
        return TorrentFileListModel::data(index, role);

    if (!tc || !index.isValid() || index.row() >= rowCount(QModelIndex()))
        return QVariant();

    if (role == Qt::ForegroundRole && index.column() == 2 && tc->getStats().multi_file_torrent) {
        const bt::TorrentFileInterface *file = &tc->getTorrentFile(index.row());
        switch (file->getPriority()) {
        case FIRST_PREVIEW_PRIORITY:
        case FIRST_PRIORITY:
            return InfoWidgetPluginSettings::firstColor();
        case LAST_PREVIEW_PRIORITY:
        case LAST_PRIORITY:
            return InfoWidgetPluginSettings::lastColor();
        case NORMAL_PREVIEW_PRIORITY:
        case NORMAL_PRIORITY:
            return QVariant();
        case ONLY_SEED_PRIORITY:
        case EXCLUDED:
        default:
            return QVariant();
        }
    }

    if (role == Qt::DisplayRole)
        return displayData(index);
    else if (role == Qt::UserRole)
        return sortData(index);

    return QVariant();
}

QVariant IWFileListModel::displayData(const QModelIndex &index) const
{
    if (tc->getStats().multi_file_torrent) {
        const bt::TorrentFileInterface *file = &tc->getTorrentFile(index.row());
        switch (index.column()) {
        case 2:
            return PriorityString(file);
        case 3:
            if (file->isMultimedia()) {
                if (file->isPreviewAvailable())
                    return i18nc("Preview available", "Available");
                else
                    return i18nc("Preview pending", "Pending");
            } else
                return i18nc("No preview available", "No");
        case 4: {
            float percent = file->getDownloadPercentage();
            return ki18n("%1 %").subs(percent, 0, 'f', 2).toString();
        }
        default:
            return QVariant();
        }
    } else {
        switch (index.column()) {
        case 2:
            return QVariant();
        case 3:
            if (mmfile) {
                if (tc->readyForPreview())
                    return i18nc("Preview available", "Available");
                else
                    return i18nc("Preview pending", "Pending");
            } else
                return i18nc("No preview available", "No");
        case 4: {
            double percent = bt::Percentage(tc->getStats());
            return ki18n("%1 %").subs(percent, 0, 'f', 2).toString();
        }
        default:
            return QVariant();
        }
    }
    return QVariant();
}

QVariant IWFileListModel::sortData(const QModelIndex &index) const
{
    if (tc->getStats().multi_file_torrent) {
        const bt::TorrentFileInterface *file = &tc->getTorrentFile(index.row());
        switch (index.column()) {
        case 2:
            return (int)file->getPriority();
        case 3:
            if (file->isMultimedia()) {
                if (file->isPreviewAvailable())
                    return 3;
                else
                    return 2;
            } else
                return 1;
        case 4:
            return file->getDownloadPercentage();
        }
    } else {
        switch (index.column()) {
        case 2:
            return QVariant();
        case 3:
            if (mmfile) {
                if (tc->readyForPreview())
                    return 3;
                else
                    return 2;
            } else
                return 1;
        case 4:
            return bt::Percentage(tc->getStats());
        }
    }
    return QVariant();
}

bool IWFileListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::CheckStateRole)
        return TorrentFileListModel::setData(index, value, role);
    else if (role == Qt::EditRole) {
        if (value.toString().isEmpty())
            return false;

        if (tc->getStats().multi_file_torrent) {
            bt::TorrentFileInterface &file = tc->getTorrentFile(index.row());

            QFileInfo fi(file.getPathOnDisk());
            QString path = fi.absolutePath();
            if (!path.endsWith(bt::DirSeparator())) {
                path += bt::DirSeparator();
            }
            path += value.toString();

            // Check if there is a sibling with the same name
            bt::Uint32 num_files = tc->getNumFiles();
            for (bt::Uint32 i = 0; i < num_files; i++) {
                if ((int)i == index.row())
                    continue;

                if (path == tc->getTorrentFile(i).getPathOnDisk())
                    return false;
            }

            // keep track of modified paths
            file.setPathOnDisk(path);
            // remove the local download path since user_modified_path
            // is a relative path inside the torrent
            path = path.remove(tc->getStats().output_path);
            file.setUserModifiedPath(path);
        } else {
            // change the name of the file or toplevel directory
            QString path = value.toString();
            tc->setUserModifiedFileName(path);
        }
        static_cast<TorrentControl*>(tc)->afterRename();
        Q_EMIT dataChanged(index, index);
        return true;
    }

    if (!tc || !index.isValid() || role != Qt::UserRole)
        return false;

    int r = index.row();
    if (r < 0 || r >= rowCount(QModelIndex()))
        return false;

    bt::TorrentFileInterface &file = tc->getTorrentFile(r);
    ;
    Priority prio = (bt::Priority)value.toInt();
    Priority old = file.getPriority();

    if (prio != old) {
        file.setPriority(prio);
        Q_EMIT dataChanged(createIndex(index.row(), 0), createIndex(index.row(), 4));
    }

    return true;
}

void IWFileListModel::filePercentageChanged(bt::TorrentFileInterface *file, float percentage)
{
    Q_UNUSED(percentage);
    if (!tc)
        return;

    QModelIndex idx = createIndex(file->getIndex(), 4, file);
    Q_EMIT dataChanged(idx, idx);
}

void IWFileListModel::filePreviewChanged(bt::TorrentFileInterface *file, bool preview)
{
    Q_UNUSED(preview);
    if (!tc)
        return;

    QModelIndex idx = createIndex(file->getIndex(), 3, file);
    Q_EMIT dataChanged(idx, idx);
}

void IWFileListModel::update()
{
    if (!tc)
        return;

    if (!tc->getStats().multi_file_torrent) {
        bool changed = false;
        bool np = mmfile && tc->readyForPreview();
        if (preview != np) {
            preview = np;
            changed = true;
        }

        double perc = bt::Percentage(tc->getStats());
        if (fabs(perc - percentage) > 0.001) {
            percentage = perc;
            changed = true;
        }

        if (changed)
            Q_EMIT dataChanged(createIndex(0, 0), createIndex(0, 4));
    }
}
}

#include "moc_iwfilelistmodel.cpp"
