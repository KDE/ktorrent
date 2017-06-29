/***************************************************************************
 *   Copyright (C) 2006 by Ivan Vasić                                     *
 *   ivasic@gmail.com                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/
#include "logflags.h"

#include <QString>

#include <KSharedConfig>
#include <KLocalizedString>

#include <util/log.h>
#include <util/logsystemmanager.h>
#include <torrent/globals.h>

#include "logviewer.h"
#include "logviewerpluginsettings.h"

using namespace bt;

namespace kt
{

    LogFlags::LogFlags()
    {
        updateFlags();
        LogSystemManager& lsman = LogSystemManager::instance();
        connect(&lsman, SIGNAL(registered(const QString&)), this, SLOT(registered(const QString&)));
        connect(&lsman, SIGNAL(unregisted(const QString&)), this, SLOT(unregistered(const QString&)));
    }

    LogFlags::~LogFlags()
    {}

    bool LogFlags::checkFlags(unsigned int arg)
    {
        QList<LogFlag>::iterator i = log_flags.begin();
        while (i != log_flags.end())
        {
            const LogFlag& f = *i;
            if (f.id & arg)
                return f.flag & arg;
            i++;
        }

        return false;
    }

    void LogFlags::updateFlags()
    {
        KConfigGroup cfg = KSharedConfig::openConfig()->group("LogFlags");
        log_flags.clear();
        LogSystemManager& lsman = LogSystemManager::instance();
        for (LogSystemManager::iterator i = lsman.begin(); i != lsman.end(); i++)
        {
            LogFlag f;
            f.name = i.key();
            f.id = i.value();
            f.flag = cfg.readEntry(QStringLiteral("sys_%1").arg(i.value()), LOG_ALL);
            log_flags.append(f);
        }
    }

    QString LogFlags::getFormattedMessage(unsigned int arg, const QString& line)
    {
        if ((arg & LOG_ALL) == LOG_ALL)
            return line;

        if (arg & 0x04) // Debug
        {
            return QStringLiteral("<font color=\"#646464\">%1</font>").arg(line);
        }

        if (arg & 0x02) // Notice
            return line;

        if (arg & 0x01) // Important
        {
            return QStringLiteral("<b>%1</b>").arg(line);
        }

        return line;
    }

    int LogFlags::rowCount(const QModelIndex& parent) const
    {
        if (!parent.isValid())
            return log_flags.count();
        else
            return 0;
    }

    int LogFlags::columnCount(const QModelIndex& parent) const
    {
        if (!parent.isValid())
            return 2;
        else
            return 0;
    }

    QVariant LogFlags::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
            return QVariant();

        switch (section)
        {
        case 0: return i18n("System");
        case 1: return i18n("Log Level");
        default:
            return QVariant();
        }
    }

    QVariant LogFlags::data(const QModelIndex& index, int role) const
    {
        if (!index.isValid())
            return QVariant();

        if (role == Qt::DisplayRole)
        {
            const LogFlag& f = log_flags.at(index.row());
            switch (index.column())
            {
            case 0: return f.name;
            case 1: return flagToString(f.flag);
            default: return QVariant();
            }
        }
        else if (role == Qt::EditRole && index.column() == 1)
        {
            const LogFlag& f = log_flags.at(index.row());
            return f.flag;
        }

        return QVariant();
    }

    bool LogFlags::setData(const QModelIndex& index, const QVariant& value, int role)
    {
        if (!index.isValid() || role != Qt::EditRole || index.column() != 1)
            return false;

        bt::Uint32 flag = value.toUInt();
        if (flag != LOG_ALL && flag != LOG_NONE && flag != LOG_DEBUG && flag != LOG_NOTICE && flag != LOG_IMPORTANT)
            return false;

        LogFlag& f = log_flags[index.row()];
        f.flag = flag;

        KConfigGroup cfg = KSharedConfig::openConfig()->group("LogFlags");
        cfg.writeEntry(QStringLiteral("sys_%1").arg(f.id), flag);
        cfg.sync();

        emit dataChanged(index, index);
        return true;
    }

    Qt::ItemFlags LogFlags::flags(const QModelIndex& index) const
    {
        if (!index.isValid())
            return Qt::ItemIsEnabled;

        if (index.column() == 1)
            return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
        else
            return QAbstractItemModel::flags(index);
    }

    bool LogFlags::removeRows(int row, int count, const QModelIndex& parent)
    {
        Q_UNUSED(parent);

        beginRemoveRows(QModelIndex(), row, row + count - 1);
        endRemoveRows();
        return true;
    }

    bool LogFlags::insertRows(int row, int count, const QModelIndex& parent)
    {
        Q_UNUSED(parent);

        beginInsertRows(QModelIndex(), row, row + count - 1);
        endInsertRows();
        return true;
    }

    QString LogFlags::flagToString(bt::Uint32 flag) const
    {
        switch (flag)
        {
        case LOG_DEBUG: return i18n("Debug");
        case LOG_NOTICE: return i18n("Notice");
        case LOG_IMPORTANT: return i18n("Important");
        case LOG_ALL: return i18n("All");
        case LOG_NONE: return i18n("None");
        default: return QString();
        }
    }

    void LogFlags::registered(const QString& sys)
    {
        KConfigGroup cfg = KSharedConfig::openConfig()->group("LogFlags");

        LogSystemManager& lsman = LogSystemManager::instance();
        LogFlag f;
        f.id = lsman.systemID(sys);
        f.flag = cfg.readEntry(QStringLiteral("sys_%1").arg(f.id), LOG_ALL);;
        f.name = sys;
        log_flags.append(f);
        insertRow(log_flags.count() - 1);
    }

    void LogFlags::unregistered(const QString& sys)
    {
        int idx = 0;
        foreach (const LogFlag& f, log_flags)
        {
            if (sys == f.name)
            {
                removeRow(idx);
                log_flags.removeAt(idx);
                break;
            }
            idx++;
        }
    }
}
