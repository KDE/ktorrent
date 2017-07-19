/***************************************************************************
 *   Copyright (C) 2007 by Joris Guisson and Ivan Vasic                    *
 *   joris.guisson@gmail.com                                               *
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
#include "queuemanagermodel.h"

#include <QApplication>
#include <QColor>
#include <QIcon>
#include <QLocale>
#include <QMimeData>

#include <KLocalizedString>

#include <util/log.h>
#include <util/functions.h>
#include <torrent/queuemanager.h>
#include <interfaces/torrentinterface.h>
#include "settings.h"


using namespace bt;

namespace kt
{


    QueueManagerModel::QueueManagerModel(QueueManager* qman, QObject* parent)
        : QAbstractTableModel(parent),
          qman(qman),
          show_uploads(true),
          show_downloads(true),
          show_not_queud(true)
    {
        connect(qman, &QueueManager::queueOrdered, this, &QueueManagerModel::onQueueOrdered);
        for (QueueManager::iterator i = qman->begin(); i != qman->end(); i++)
        {
            bt::TorrentInterface* tc = *i;
            connect(tc, &bt::TorrentInterface::statusChanged, this, &QueueManagerModel::onTorrentStatusChanged);

            if (visible(tc))
            {
                Item item = {tc, 0};
                queue.append(item);
            }
        }

        //dumpQueue();
    }


    QueueManagerModel::~QueueManagerModel()
    {}

    void QueueManagerModel::onQueueOrdered()
    {
        updateQueue();
    }

    void QueueManagerModel::softReset()
    {
        emit dataChanged(index(0, 0), index(queue.count() - 1, columnCount(QModelIndex()) - 1));
    }


    void QueueManagerModel::updateQueue()
    {
        int count = queue.count();
        queue.clear();

        for (QueueManager::iterator i = qman->begin(); i != qman->end(); i++)
        {
            bt::TorrentInterface* tc = *i;
            if (visible(tc))
            {
                Item item = {tc, 0};
                queue.append(item);
            }
        }

        if (count == queue.count())
        {
            softReset();
        }
        else if (queue.count() > count)
        {
            insertRows(0, queue.count() - count, QModelIndex());
            softReset();
        }
        else // queue.count() < count)
        {
            removeRows(0, count - queue.count(), QModelIndex());
            softReset();
        }
    }

    void QueueManagerModel::setShowDownloads(bool on)
    {
        show_downloads = on;
        updateQueue();
    }

    void QueueManagerModel::setShowUploads(bool on)
    {
        show_uploads = on;
        updateQueue();
    }

    void QueueManagerModel::setShowNotQueued(bool on)
    {
        show_not_queud = on;
        updateQueue();
    }

    void QueueManagerModel::onTorrentAdded(bt::TorrentInterface* tc)
    {
        connect(tc, &bt::TorrentInterface::statusChanged, this, &QueueManagerModel::onTorrentStatusChanged);
    }

    void QueueManagerModel::onTorrentRemoved(bt::TorrentInterface* tc)
    {
        disconnect(tc, SIGNAL(statusChanged(bt::TorrentInterface*)),
                   this, SLOT(onTorrentStatusChanged(bt::TorrentInterface*)));
    }

    void QueueManagerModel::onTorrentStatusChanged(bt::TorrentInterface* tc)
    {
        int r = 0;
        foreach (const Item& i, queue)
        {
            if (tc == i.tc)
            {
                if (!visible(tc))
                {
                    queue.removeAt(r);
                    removeRow(r);
                }
                else
                {
                    QModelIndex idx = index(r, 2);
                    emit dataChanged(idx, idx);
                }
                return;
            }

            r++;
        }

        if (visible(tc))
        {
            updateQueue();
        }
    }

    int QueueManagerModel::rowCount(const QModelIndex& parent) const
    {
        if (parent.isValid())
            return 0;
        else
            return queue.count();
    }

    int QueueManagerModel::columnCount(const QModelIndex& parent) const
    {
        if (parent.isValid())
            return 0;
        else
            return 4;
    }

    QVariant QueueManagerModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
            return QVariant();

        switch (section)
        {
        case 0:
            return i18n("Order");
        case 1:
            return i18n("Name");
        case 2:
            return i18n("Status");
        case 3:
            return i18n("Time Stalled");
        case 4:
            return i18n("Priority");
        default:
            return QVariant();
        }
    }

    QVariant QueueManagerModel::data(const QModelIndex& index, int role) const
    {
        if (!index.isValid() || index.row() >= queue.count() || index.row() < 0)
            return QVariant();

        const bt::TorrentInterface* tc = queue.at(index.row()).tc;
        if (role == Qt::ForegroundRole)
        {
            if (index.column() == 2)
            {
                if (tc->getStats().running)
                    return QColor(40, 205, 40); // green
                else if (tc->getStats().status == bt::QUEUED)
                    return QColor(255, 174, 0); // yellow
                else
                    return QVariant();
            }
            return QVariant();
        }
        else if (role == Qt::DisplayRole)
        {
            switch (index.column())
            {
            case 0:
                return index.row() + 1;
            case 1:
                return tc->getDisplayName();
            case 2:
                if (tc->getStats().running)
                    return i18n("Running");
                else if (tc->getStats().status == bt::QUEUED)
                    return i18n("Queued");
                else
                    return i18n("Not queued");
                break;
            case 3:
            {
                if (!tc->getStats().running)
                    return QVariant();

                Int64 stalled_time =  queue.at(index.row()).stalled_time;
                if (stalled_time >= 1)
                    return i18n("%1", DurationToString(stalled_time));
                else
                    return QVariant();
            }
            break;
            case 4:
                return tc->getPriority();
            default:
                return QVariant();
            }
        }
        else if (role == Qt::ToolTipRole && index.column() == 0)
        {
            return i18n("Order of a torrent in the queue.\n"
                        "Use drag and drop or the move up and down buttons on the right to change the order.");
        }
        else if (role == Qt::DecorationRole && index.column() == 1)
        {
            if (!tc->getStats().completed)
                return QIcon::fromTheme(QStringLiteral("arrow-down"));
            else
                return QIcon::fromTheme(QStringLiteral("arrow-up"));
        }
        else if (role == Qt::FontRole && !search_text.isEmpty())
        {
            QFont f = QApplication::font();
            if (tc->getDisplayName().contains(search_text, Qt::CaseInsensitive))
                f.setBold(true);

            return f;
        }

        return QVariant();
    }

    Qt::ItemFlags QueueManagerModel::flags(const QModelIndex& index) const
    {
        Qt::ItemFlags defaultFlags = QAbstractTableModel::flags(index);

        if (index.isValid())
            return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
        else
            return Qt::ItemIsDropEnabled | defaultFlags;
    }

    Qt::DropActions QueueManagerModel::supportedDropActions() const
    {
        return Qt::CopyAction | Qt::MoveAction;
    }

    QStringList QueueManagerModel::mimeTypes() const
    {
        QStringList types;
        types << QStringLiteral("application/vnd.text.list");
        return types;
    }

    QMimeData* QueueManagerModel::mimeData(const QModelIndexList& indexes) const
    {
        QMimeData* mimeData = new QMimeData();
        QByteArray encodedData;

        dragged_items.clear();

        for (const QModelIndex& index : indexes)
        {
            if (index.isValid() && !dragged_items.contains(index.row()))
                dragged_items.append(index.row());
        }

        mimeData->setData(QStringLiteral("application/vnd.text.list"), QByteArrayLiteral("stuff"));
        return mimeData;
    }

    bool QueueManagerModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
    {
        Q_UNUSED(column);
        if (action == Qt::IgnoreAction)
            return true;

        if (!data->hasFormat(QStringLiteral("application/vnd.text.list")))
            return false;

        int begin_row = row;
        if (row != -1)
        {
            begin_row = row;
        }
        else if (parent.isValid())
        {
            begin_row = parent.row();
        }
        else
        {
            moveBottom(dragged_items.front(), dragged_items.count());
            return true;
        }

        int from = dragged_items.front();
        int count = dragged_items.count();
        if (from < begin_row)
        {
            while (from < begin_row)
            {
                for (int i = count - 1; i >= 0; i--)
                    swapItems(from + i, from + i + 1);
                from++;
            }
        }
        else
        {
            while (from > begin_row)
            {
                for (int i = 0; i < count; i++)
                    swapItems(from + i, from + i - 1);
                from--;
            }
        }


        updatePriorities();
        // reorder the queue
        qman->orderQueue();
        endResetModel();
        return true;
    }

    bool QueueManagerModel::removeRows(int row, int count, const QModelIndex& parent)
    {
        Q_UNUSED(parent);
        beginInsertRows(QModelIndex(), row, row + count - 1);
        endInsertRows();
        return true;
    }

    bool QueueManagerModel::insertRows(int row, int count, const QModelIndex& parent)
    {
        Q_UNUSED(parent);
        beginInsertRows(QModelIndex(), row, row + count - 1);
        endInsertRows();
        return true;
    }

    void QueueManagerModel::moveUp(int row, int count)
    {
        if (row <= 0 || row > qman->count())
            return;

        for (int i = 0; i < count; i++)
        {
            swapItems(row + i, row + i - 1);
        }

        updatePriorities();
        //dumpQueue();
        // reorder the queue
        qman->orderQueue();
        endResetModel();
    }

    void QueueManagerModel::moveDown(int row, int count)
    {
        if (row < 0 || row >= qman->count() - 1)
            return;

        for (int i = count - 1; i >= 0; i--)
        {
            swapItems(row + i, row + i + 1);
        }

        updatePriorities();
        //dumpQueue();
        // reorder the queue
        qman->orderQueue();
        endResetModel();
    }

    void QueueManagerModel::moveTop(int row, int count)
    {
        if (row < 0 || row >= qman->count())
            return;

        while (row > 0)
        {
            for (int i = 0; i < count; i++)
            {
                swapItems(row + i, row + i - 1);
            }
            row--;
        }

        updatePriorities();
        //dumpQueue();
        // reorder the queue
        qman->orderQueue();
        endResetModel();
    }

    void QueueManagerModel::moveBottom(int row, int count)
    {
        if (row < 0 || row >= qman->count())
            return;

        while (row + count < queue.count())
        {
            for (int i = count - 1; i >= 0; i--)
            {
                swapItems(row + i, row + i + 1);
            }
            row++;
        }

        updatePriorities();
        //dumpQueue();
        // reorder the queue
        qman->orderQueue();
        endResetModel();
    }

    void QueueManagerModel::dumpQueue()
    {
        int idx = 0;
        foreach (const Item& item, queue)
        {
            Out(SYS_GEN | LOG_DEBUG) << "Item " << idx << ": " << item.tc->getDisplayName() << " " << item.tc->getPriority() << endl;
            idx++;
        }
    }

    void QueueManagerModel::updatePriorities()
    {
        int idx = queue.size();
        for (QList<Item>::iterator i = queue.begin(); i != queue.end(); i++)
            i->tc->setPriority(idx--);
    }

    void QueueManagerModel::update()
    {
        TimeStamp now = bt::CurrentTime();
        int r = 0;
        for (QList<Item>::iterator i = queue.begin(); i != queue.end(); i++)
        {
            bt::TorrentInterface* tc = i->tc;
            if (!tc->getStats().running)
            {
                if (i->stalled_time != -1)
                {
                    i->stalled_time = -1;
                    emit dataChanged(createIndex(r, 3), createIndex(r, 3));
                }
            }
            else
            {
                Int64 stalled_time = 0;
                if (tc->getStats().completed)
                    stalled_time = (now - tc->getStats().last_upload_activity_time) / 1000;
                else
                    stalled_time = (now - tc->getStats().last_download_activity_time) / 1000;

                if (i->stalled_time != stalled_time)
                {
                    i->stalled_time = stalled_time;
                    emit dataChanged(createIndex(r, 3), createIndex(r, 3));
                }
            }
            r++;
        }
    }


    QModelIndex QueueManagerModel::find(const QString& text)
    {
        search_text = text;
        if (text.isEmpty())
        {
            endResetModel();
            return QModelIndex();
        }

        int idx = 0;
        foreach (const Item& i, queue)
        {
            bt::TorrentInterface* tc = i.tc;
            if (tc->getDisplayName().contains(text, Qt::CaseInsensitive))
            {
                endResetModel();
                return index(idx, 0);
            }
            idx++;
        }

        endResetModel();
        return QModelIndex();
    }

    bool QueueManagerModel::visible(const bt::TorrentInterface* tc)
    {
        if (!show_uploads && tc->getStats().completed)
            return false;

        if (!show_downloads && !tc->getStats().completed)
            return false;

        if (!show_not_queud && !tc->isAllowedToStart())
            return false;

        return true;
    }

    void QueueManagerModel::swapItems(int a, int b)
    {
        if (a < 0 || a >= queue.count() || b < 0 || b >= queue.count())
            return;

        queue.swap(a, b);
    }


}

