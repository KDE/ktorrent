/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "webseedsmodel.h"

#include <KLocalizedString>

#include <interfaces/webseedinterface.h>
#include <util/functions.h>

using namespace bt;

namespace kt
{
WebSeedsModel::WebSeedsModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

WebSeedsModel::~WebSeedsModel()
{
}

void WebSeedsModel::changeTC(bt::TorrentInterface *tc)
{
    curr_tc = tc;
    beginResetModel();
    items.clear();
    if (tc) {
        for (Uint32 i = 0; i < tc->getNumWebSeeds(); i++) {
            const bt::WebSeedInterface *ws = tc->getWebSeed(i);
            Item item;
            item.status = ws->getStatus();
            item.downloaded = ws->getTotalDownloaded();
            item.speed = ws->getDownloadRate();
            items.append(item);
        }
    }
    endResetModel();
}

bool WebSeedsModel::update()
{
    if (!curr_tc)
        return false;

    bt::TorrentInterface *tc = curr_tc.data();
    bool ret = false;

    for (Uint32 i = 0; i < tc->getNumWebSeeds(); i++) {
        const bt::WebSeedInterface *ws = tc->getWebSeed(i);
        Item &item = items[i];
        bool changed = false;
        if (item.status != ws->getStatus()) {
            changed = true;
            item.status = ws->getStatus();
        }

        if (item.downloaded != ws->getTotalDownloaded()) {
            changed = true;
            item.downloaded = ws->getTotalDownloaded();
        }

        if (item.speed != ws->getDownloadRate()) {
            changed = true;
            item.speed = ws->getDownloadRate();
        }

        if (changed) {
            Q_EMIT dataChanged(createIndex(i, 1), createIndex(i, 3));
            ret = true;
        }
    }

    return ret;
}

int WebSeedsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    else
        return curr_tc ? curr_tc.data()->getNumWebSeeds() : 0;
}

int WebSeedsModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    else
        return 4;
}

QVariant WebSeedsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
        return QVariant();

    switch (section) {
    case 0:
        return i18n("URL");
    case 1:
        return i18n("Speed");
    case 2:
        return i18n("Downloaded");
    case 3:
        return i18n("Status");
    default:
        return QVariant();
    }
}

QVariant WebSeedsModel::data(const QModelIndex &index, int role) const
{
    if (!curr_tc)
        return QVariant();

    if (!index.isValid() || index.row() >= (int)curr_tc.data()->getNumWebSeeds() || index.row() < 0)
        return QVariant();

    if (role == Qt::DisplayRole) {
        const bt::WebSeedInterface *ws = curr_tc.data()->getWebSeed(index.row());
        switch (index.column()) {
        case 0:
            return ws->getUrl().toDisplayString();
        case 1:
            return bt::BytesPerSecToString(ws->getDownloadRate());
        case 2:
            return bt::BytesToString(ws->getTotalDownloaded());
        case 3:
            return ws->getStatus();
        }
    } else if (role == Qt::CheckStateRole && index.column() == 0) {
        const bt::WebSeedInterface *ws = curr_tc.data()->getWebSeed(index.row());
        return ws->isEnabled() ? Qt::Checked : Qt::Unchecked;
    } else if (role == Qt::UserRole) {
        const bt::WebSeedInterface *ws = curr_tc.data()->getWebSeed(index.row());
        switch (index.column()) {
        case 0:
            return ws->getUrl().toDisplayString();
        case 1:
            return ws->getDownloadRate();
        case 2:
            return ws->getTotalDownloaded();
        case 3:
            return ws->getStatus();
        }
    }
    return QVariant();
}

Qt::ItemFlags WebSeedsModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QAbstractTableModel::flags(index);
    if (index.column() == 0)
        flags |= Qt::ItemIsUserCheckable;

    return flags;
}

bool WebSeedsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!curr_tc || role != Qt::CheckStateRole)
        return false;

    if (!index.isValid() || index.row() >= (int)curr_tc.data()->getNumWebSeeds() || index.row() < 0)
        return false;

    bt::WebSeedInterface *ws = curr_tc.data()->getWebSeed(index.row());
    ws->setEnabled((Qt::CheckState)value.toInt() == Qt::Checked), Q_EMIT dataChanged(index, index);
    return true;
}
}

#include "moc_webseedsmodel.cpp"
