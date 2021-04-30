/*
    SPDX-FileCopyrightText: 2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2007 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "viewmodel.h"

#include <algorithm>
#include <cmath>

#include <QBrush>
#include <QColor>
#include <QIcon>
#include <QLocale>
#include <QMimeData>
#include <QPalette>

#include <KLocalizedString>

#include <groups/group.h>
#include <interfaces/torrentinterface.h>
#include <torrent/queuemanager.h>
#include <torrent/timeestimator.h>
#include <util/functions.h>
#include <util/sha1hash.h>

#include "core.h"
#include "settings.h"
#include "view.h"
#include "viewdelegate.h"

using namespace bt;

namespace kt
{
ViewModel::Item::Item(bt::TorrentInterface *tc)
    : tc(tc)
{
    const TorrentStats &s = tc->getStats();
    status = s.status;
    bytes_downloaded = s.bytes_downloaded;
    total_bytes_to_download = s.total_bytes_to_download;
    bytes_uploaded = s.bytes_uploaded;
    bytes_left = s.bytes_left_to_download;
    download_rate = s.download_rate;
    upload_rate = s.upload_rate;
    eta = tc->getETA();
    seeders_connected_to = s.seeders_connected_to;
    seeders_total = s.seeders_total;
    leechers_total = s.leechers_total;
    leechers_connected_to = s.leechers_connected_to;
    percentage = Percentage(s);
    share_ratio = s.shareRatio();
    runtime_dl = tc->getRunningTimeDL();
    runtime_ul = tc->getRunningTimeUL() - tc->getRunningTimeDL();
    hidden = false;
    time_added = s.time_added;
    highlight = false;
}

bool ViewModel::Item::update(int row, int sort_column, QModelIndexList &to_update, kt::ViewModel *model)
{
    bool ret = false;
    const TorrentStats &s = tc->getStats();

    const auto update_if_differs = [&](auto &target, const auto &source, int column) {
        if (target != source) {
            to_update.append(model->index(row, column));
            target = source;
            ret |= (sort_column == column);
        }
    };

    const auto update_if_differs_float = [&](auto &target, const auto &source, int column) {
        if (fabs(target - source) > 0.001) {
            to_update.append(model->index(row, column));
            target = source;
            ret |= (sort_column == column);
        }
    };

    update_if_differs(status, s.status, NAME);
    update_if_differs(bytes_downloaded, s.bytes_downloaded, BYTES_DOWNLOADED);
    update_if_differs(total_bytes_to_download, s.total_bytes_to_download, TOTAL_BYTES_TO_DOWNLOAD);
    update_if_differs(bytes_uploaded, s.bytes_uploaded, BYTES_UPLOADED);
    update_if_differs(bytes_left, s.bytes_left, BYTES_LEFT);
    update_if_differs(download_rate, s.download_rate, DOWNLOAD_RATE);
    update_if_differs(upload_rate, s.upload_rate, UPLOAD_RATE);
    update_if_differs(eta, tc->getETA(), ETA);
    update_if_differs(seeders_connected_to, s.seeders_connected_to, SEEDERS);
    update_if_differs(seeders_total, s.seeders_total, SEEDERS);
    update_if_differs(leechers_connected_to, s.leechers_connected_to, LEECHERS);
    update_if_differs(leechers_total, s.leechers_total, LEECHERS);

    update_if_differs_float(percentage, Percentage(s), PERCENTAGE);
    update_if_differs_float(share_ratio, s.shareRatio(), SHARE_RATIO);

    update_if_differs(runtime_dl, tc->getRunningTimeDL(), DOWNLOAD_TIME);
    // clang-format off
    const auto rul = (tc->getRunningTimeUL() >= tc->getRunningTimeDL()
                      ? tc->getRunningTimeUL() - tc->getRunningTimeDL()
                      : 0);
    // clang-format on
    update_if_differs(runtime_ul, rul, SEED_TIME);

    return ret;
}

QVariant ViewModel::Item::data(int col) const
{
    static QLocale locale;
    const TorrentStats &s = tc->getStats();
    switch (col) {
    case NAME:
        return tc->getDisplayName();
    case BYTES_DOWNLOADED:
        return BytesToString(bytes_downloaded);
    case TOTAL_BYTES_TO_DOWNLOAD:
        return BytesToString(total_bytes_to_download);
    case BYTES_UPLOADED:
        return BytesToString(bytes_uploaded);
    case BYTES_LEFT:
        return bytes_left > 0 ? BytesToString(bytes_left) : QVariant();
    case DOWNLOAD_RATE:
        if (download_rate >= 103 && s.bytes_left_to_download > 0) // lowest "visible" speed, all below will be 0,0 Kb/s
            return BytesPerSecToString(download_rate);
        else
            return QVariant();
    case UPLOAD_RATE:
        if (upload_rate >= 103) // lowest "visible" speed, all below will be 0,0 Kb/s
            return BytesPerSecToString(upload_rate);
        else
            return QVariant();
    case ETA:
        if (eta == bt::TimeEstimator::NEVER)
            return QString(QChar(0x221E)); // infinity
        else if (eta != bt::TimeEstimator::ALREADY_FINISHED)
            return DurationToString(eta);
        else
            return QVariant();
    case SEEDERS:
        return QString(QString::number(seeders_connected_to) + QLatin1String(" (") + QString::number(seeders_total) + QLatin1Char(')'));
    case LEECHERS:
        return QString(QString::number(leechers_connected_to) + QLatin1String(" (") + QString::number(leechers_total) + QLatin1Char(')'));
    // xgettext: no-c-format
    case PERCENTAGE:
        return percentage;
    case SHARE_RATIO:
        return locale.toString(share_ratio, 'f', 2);
    case DOWNLOAD_TIME:
        return DurationToString(runtime_dl);
    case SEED_TIME:
        return DurationToString(runtime_ul);
    case DOWNLOAD_LOCATION:
        return tc->getStats().output_path;
    case TIME_ADDED:
        return locale.toString(time_added);
    default:
        return QVariant();
    }
}

bool ViewModel::Item::lessThan(int col, const Item *other) const
{
    switch (col) {
    case NAME:
        return QString::localeAwareCompare(tc->getDisplayName(), other->tc->getDisplayName()) < 0;
    case BYTES_DOWNLOADED:
        return bytes_downloaded < other->bytes_downloaded;
    case TOTAL_BYTES_TO_DOWNLOAD:
        return total_bytes_to_download < other->total_bytes_to_download;
    case BYTES_UPLOADED:
        return bytes_uploaded < other->bytes_uploaded;
    case BYTES_LEFT:
        return bytes_left < other->bytes_left;
    case DOWNLOAD_RATE:
        return (download_rate < 102 ? 0 : download_rate) < (other->download_rate < 102 ? 0 : other->download_rate);
    case UPLOAD_RATE:
        return (upload_rate < 102 ? 0 : upload_rate) < (other->upload_rate < 102 ? 0 : other->upload_rate);
    case ETA:
        return eta < other->eta;
    case SEEDERS:
        if (seeders_connected_to == other->seeders_connected_to)
            return seeders_total < other->seeders_total;
        else
            return seeders_connected_to < other->seeders_connected_to;
    case LEECHERS:
        if (leechers_connected_to == other->leechers_connected_to)
            return leechers_total < other->leechers_total;
        else
            return leechers_connected_to < other->leechers_connected_to;
    case PERCENTAGE:
        return percentage < other->percentage;
    case SHARE_RATIO:
        return share_ratio < other->share_ratio;
    case DOWNLOAD_TIME:
        return runtime_dl < other->runtime_dl;
    case SEED_TIME:
        return runtime_ul < other->runtime_ul;
    case DOWNLOAD_LOCATION:
        return tc->getStats().output_path < other->tc->getStats().output_path;
    case TIME_ADDED:
        return time_added < other->time_added;
    default:
        return false;
    }
}

QVariant ViewModel::Item::color(int col) const
{
    if (col == NAME) {
        switch (status) {
        case bt::SEEDING:
        case bt::SUPERSEEDING:
        case bt::DOWNLOADING:
        case bt::ALLOCATING_DISKSPACE:
        case bt::STALLED:
        case bt::CHECKING_DATA: {
            if (Settings::highlightTorrentNameByTrackerStatus()) {
                // apply additional highlighting to torrent names
                const bt::TrackersStatusInfo tsi = tc->getTrackersList()->getTrackersStatusInfo();
                if (tsi.trackers_count) {
                    if ((tsi.errors + tsi.warnings) == tsi.trackers_count) {
                        // no any OK statuses
                        if (tsi.timeout_errors)
                            return Settings::timeoutTrackerConnectionColor();
                        if (tsi.warnings)
                            return Settings::warningsTrackerConnectionColor();
                        return Settings::noTrackerConnectionColor();
                    }
                }
            }
            return (status == bt::STALLED || bt::STALLED == bt::CHECKING_DATA) ? Settings::stalledTorrentColor() : Settings::okTorrentColor();
        }
        case bt::ERROR:
            return Settings::errorTorrentColor();
        case bt::NOT_STARTED:
        case bt::STOPPED:
        case bt::QUEUED:
        case bt::DOWNLOAD_COMPLETE:
        case bt::SEEDING_COMPLETE:
        default:
            return QVariant();
        }

    } else if (col == SHARE_RATIO) {
        return share_ratio >= Settings::greenRatio() ? Settings::goodShareRatioColor() : Settings::lowShareRatioColor();
    } else
        return QVariant();
}

bool ViewModel::Item::visible(Group *group, const QString &filter_string) const
{
    if (group && !group->isMember(tc))
        return false;

    return filter_string.isEmpty() || tc->getDisplayName().contains(filter_string, Qt::CaseInsensitive);
}

QVariant ViewModel::Item::statusIcon() const
{
    switch (tc->getStats().status) {
    case NOT_STARTED:
    case STOPPED:
        return QIcon::fromTheme(QStringLiteral("kt-stop"));
    case SEEDING_COMPLETE:
    case DOWNLOAD_COMPLETE:
        return QIcon::fromTheme(QStringLiteral("task-complete"));
    case SEEDING:
    case SUPERSEEDING:
        return QIcon::fromTheme(QStringLiteral("go-up"));
    case DOWNLOADING:
        return QIcon::fromTheme(QStringLiteral("go-down"));
    case STALLED:
        if (tc->getStats().completed)
            return QIcon::fromTheme(QStringLiteral("go-up"));
        else
            return QIcon::fromTheme(QStringLiteral("go-down"));
    case ALLOCATING_DISKSPACE:
        return QIcon::fromTheme(QStringLiteral("drive-harddisk"));
    case ERROR:
    case NO_SPACE_LEFT:
        return QIcon::fromTheme(QStringLiteral("dialog-error"));
    case QUEUED:
        return QIcon::fromTheme(QStringLiteral("download-later"));
    case CHECKING_DATA:
        return QIcon::fromTheme(QStringLiteral("kt-check-data"));
    case PAUSED:
        return QIcon::fromTheme(QStringLiteral("kt-pause"));
    default:
        return QVariant();
    }
}

////////////////////////////////////////////////////////

ViewModel::ViewModel(Core *core, View *parent)
    : QAbstractTableModel(parent)
    , core(core)
    , view(parent)
{
    connect(core, &Core::aboutToQuit, this, &ViewModel::onExit); // model must be in core's thread to be notified in time
    connect(core, &Core::torrentAdded, this, &ViewModel::addTorrent);
    connect(core, &Core::torrentRemoved, this, &ViewModel::removeTorrent);
    sort_column = 0;
    sort_order = Qt::AscendingOrder;
    group = nullptr;
    num_visible = 0;

    const kt::QueueManager *const qman = core->getQueueManager();
    for (bt::TorrentInterface *i : *qman) {
        torrents.append(new Item(i));
        num_visible++;
    }
}

ViewModel::~ViewModel()
{
    qDeleteAll(torrents);
}

void ViewModel::setGroup(Group *g)
{
    group = g;
}

void ViewModel::addTorrent(bt::TorrentInterface *ti)
{
    Item *i = new Item(ti);
    if (Settings::highlightNewTorrents()) {
        i->highlight = true;

        // Turn off highlight for previously highlighted torrents
        for (Item *item : qAsConst(torrents))
            if (item->highlight)
                item->highlight = false;
    }

    torrents.append(i);
    update(view->viewDelegate(), true);

    // Scroll to new torrent
    int idx = 0;
    for (Item *item : qAsConst(torrents)) {
        if (item->tc == ti) {
            view->scrollTo(index(idx, 0));
            break;
        }
        idx++;
    }
}

void ViewModel::removeTorrent(bt::TorrentInterface *ti)
{
    int idx = 0;
    for (Item *item : qAsConst(torrents)) {
        if (item->tc == ti) {
            removeRow(idx);
            update(view->viewDelegate(), true);
            break;
        }
        idx++;
    }
}

void ViewModel::emitDataChanged(int row, int col)
{
    QModelIndex idx = createIndex(row, col);
    Q_EMIT dataChanged(idx, idx);
    // Q_EMIT dataChanged(createIndex(row,0),createIndex(row,14));
}

bool ViewModel::update(ViewDelegate *delegate, bool force_resort)
{
    update_list.clear();
    bool resort = force_resort;
    num_visible = 0;

    int row = 0;
    for (Item *i : qAsConst(torrents)) {
        bool hidden = !i->visible(group, filter_string);
        if (!hidden && i->update(row, sort_column, update_list, this))
            resort = true;

        if (hidden != i->hidden) {
            i->hidden = hidden;
            resort = true;
        }

        // hide the extender if there is one shown
        if (hidden && delegate->extended(i->tc))
            delegate->hideExtender(i->tc);

        if (!i->hidden)
            num_visible++;
        row++;
    }

    if (resort) {
        update_list.clear();
        sort(sort_column, sort_order);
        return true;
    }

    return false;
}

void ViewModel::setFilterString(const QString &filter)
{
    filter_string = filter;
}

int ViewModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    else
        return num_visible;
}

int ViewModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    else
        return _NUMBER_OF_COLUMNS;
}

QVariant ViewModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal)
        return QVariant();

    if (role == Qt::DisplayRole) {
        switch (section) {
        case NAME:
            return i18n("Name");
        case BYTES_DOWNLOADED:
            return i18n("Downloaded");
        case TOTAL_BYTES_TO_DOWNLOAD:
            return i18n("Size");
        case BYTES_UPLOADED:
            return i18n("Uploaded");
        case BYTES_LEFT:
            return i18nc("Bytes left to downloaded", "Left");
        case DOWNLOAD_RATE:
            return i18n("Down Speed");
        case UPLOAD_RATE:
            return i18n("Up Speed");
        case ETA:
            return i18n("Time Left");
        case SEEDERS:
            return i18n("Seeders");
        case LEECHERS:
            return i18n("Leechers");
        case PERCENTAGE:
            // xgettext: no-c-format
            return i18n("% Complete");
        case SHARE_RATIO:
            return i18n("Share Ratio");
        case DOWNLOAD_TIME:
            return i18n("Time Downloaded");
        case SEED_TIME:
            return i18n("Time Seeded");
        case DOWNLOAD_LOCATION:
            return i18n("Location");
        case TIME_ADDED:
            return i18n("Added");
        default:
            return QVariant();
        }
    } else if (role == Qt::ToolTipRole) {
        switch (section) {
        case BYTES_DOWNLOADED:
            return i18n("How much data we have downloaded of the torrent");
        case TOTAL_BYTES_TO_DOWNLOAD:
            return i18n("Total size of the torrent, excluded files are not included");
        case BYTES_UPLOADED:
            return i18n("How much data we have uploaded");
        case BYTES_LEFT:
            return i18n("How much data left to download");
        case DOWNLOAD_RATE:
            return i18n("Current download speed");
        case UPLOAD_RATE:
            return i18n("Current upload speed");
        case ETA:
            return i18n("How much time is left before the torrent is finished or before the maximum share ratio is reached, if that is enabled");
        case SEEDERS:
            return i18n("How many seeders we are connected to (How many seeders there are according to the tracker)");
        case LEECHERS:
            return i18n("How many leechers we are connected to (How many leechers there are according to the tracker)");
        // xgettext: no-c-format
        case PERCENTAGE:
            return i18n("The percentage of data we have of the whole torrent, not including excluded files");
        case SHARE_RATIO:
            return i18n("Share ratio is the number of bytes uploaded divided by the number of bytes downloaded");
        case DOWNLOAD_TIME:
            return i18n("How long we have been downloading the torrent");
        case SEED_TIME:
            return i18n("How long we have been seeding the torrent");
        case DOWNLOAD_LOCATION:
            return i18n("The location of the torrent's data on disk");
        case TIME_ADDED:
            return i18n("When this torrent was added");
        default:
            return QVariant();
        }
    }

    return QVariant();
}

QModelIndex ViewModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid())
        return QModelIndex();

    if (row < 0 || row >= torrents.count())
        return QModelIndex();
    else
        return createIndex(row, column, torrents[row]);
}

QVariant ViewModel::data(const QModelIndex &index, int role) const
{
    // there is no point checking index.row() < 0 because isValid already does this
    if (!index.isValid() || index.row() >= torrents.count())
        return QVariant();

    Item *item = reinterpret_cast<Item *>(index.internalPointer());
    if (!item)
        return QVariant();

    if (role == Qt::ForegroundRole) {
        return item->color(index.column());
    } else if (role == Qt::DisplayRole) {
        return item->data(index.column());
    } else if (role == Qt::EditRole && index.column() == NAME) {
        return item->tc->getDisplayName();
    } else if (role == Qt::DecorationRole && index.column() == NAME) {
        return item->statusIcon();
    } else if (role == Qt::ToolTipRole && index.column() == NAME) {
        QString tooltip;
        bt::TorrentInterface *tc = item->tc;
        if (tc->loadUrl().isValid())
            tooltip = i18n("%1<br>Url: <b>%2</b>", tc->getDisplayName(), tc->loadUrl().toDisplayString());
        else
            tooltip = tc->getDisplayName();

        tooltip += QLatin1String("<br/><br/>") + tc->getStats().statusToString();
        if (tc->getTrackersList()->noTrackersReachable())
            tooltip += i18n("<br/><br/>Unable to contact a tracker.");

        return tooltip;
    } else if (role == Qt::TextAlignmentRole) {
        switch (index.column()) {
        case NAME:
        case PERCENTAGE:
        case DOWNLOAD_LOCATION:
        case TIME_ADDED:
            return Qt::AlignLeft + Qt::AlignVCenter;
        default:
            return Qt::AlignRight + Qt::AlignVCenter;
        }
    } else if (role == Qt::FontRole && item->highlight) {
        QFont f = view->font();
        f.setBold(true);
        return f;
    }

    return QVariant();
}

bool ViewModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || index.row() >= torrents.count() || role != Qt::EditRole || index.column() != NAME)
        return false;

    QString name = value.toString();
    Item *item = reinterpret_cast<Item *>(index.internalPointer());
    if (!item)
        return false;

    bt::TorrentInterface *tc = item->tc;
    tc->setDisplayName(name);
    Q_EMIT dataChanged(index, index);
    if (sort_column == NAME)
        sort(sort_column, sort_order);
    return true;
}

Qt::ItemFlags ViewModel::flags(const QModelIndex &index) const
{
    if (!index.isValid() || index.row() >= torrents.count())
        return QAbstractTableModel::flags(index) | Qt::ItemIsDropEnabled;

    Qt::ItemFlags flags = QAbstractTableModel::flags(index) | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
    if (index.column() == NAME)
        flags |= Qt::ItemIsEditable;

    return flags;
}

QStringList ViewModel::mimeTypes() const
{
    QStringList types;
    types << QStringLiteral("application/x-ktorrent-drag-object");
    types << QStringLiteral("text/uri-list");
    return types;
}

QMimeData *ViewModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mime_data = new QMimeData();
    QByteArray encoded_data;

    QDataStream stream(&encoded_data, QIODevice::WriteOnly);
    QStringList hashes;
    for (const QModelIndex &index : indexes) {
        if (!index.isValid())
            continue;

        const bt::TorrentInterface *ti = torrentFromIndex(index);
        if (ti) {
            QString hash = ti->getInfoHash().toString();
            if (!hashes.contains(hash)) {
                hashes.append(hash);
            }
        }
    }

    for (const QString &s : qAsConst(hashes))
        stream << s;

    mime_data->setData(QStringLiteral("application/x-ktorrent-drag-object"), encoded_data);
    return mime_data;
}

bool ViewModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    Q_UNUSED(row);
    Q_UNUSED(column);
    Q_UNUSED(parent);
    if (action == Qt::IgnoreAction)
        return true;

    if (!data->hasUrls())
        return false;

    const QList<QUrl> files = data->urls();
    for (const QUrl &file : files) {
        core->load(file, QString());
    }

    return true;
}

Qt::DropActions ViewModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

void ViewModel::torrentsFromIndexList(const QModelIndexList &idx, QList<bt::TorrentInterface *> &tlist)
{
    for (const QModelIndex &i : idx) {
        bt::TorrentInterface *tc = torrentFromIndex(i);
        if (tc)
            tlist.append(tc);
    }
}

bt::TorrentInterface *ViewModel::torrentFromIndex(const QModelIndex &index) const
{
    if (index.isValid() && index.row() < torrents.count())
        return torrents[index.row()]->tc;
    else
        return nullptr;
}

bt::TorrentInterface *ViewModel::torrentFromRow(int index) const
{
    if (index < torrents.count() && index >= 0)
        return torrents[index]->tc;
    else
        return nullptr;
}

void ViewModel::allTorrents(QList<bt::TorrentInterface *> &tlist) const
{
    for (Item *item : qAsConst(torrents)) {
        if (item->visible(group, filter_string))
            tlist.append(item->tc);
    }
}

bool ViewModel::insertRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent);
    beginInsertRows(QModelIndex(), row, row + count - 1);
    endInsertRows();
    return true;
}

bool ViewModel::removeRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent);
    beginRemoveRows(QModelIndex(), row, row + count - 1);
    for (int i = 0; i < count; i++) {
        Item *item = torrents[row + i];
        delete item;
    }
    torrents.remove(row, count);
    endRemoveRows();
    return true;
}

void ViewModel::onExit()
{
    // items should be removed before Core delete their tc data.
    removeRows(0, rowCount(), QModelIndex());
}

class ViewModelItemCmp
{
public:
    ViewModelItemCmp(int col, Qt::SortOrder order)
        : col(col)
        , order(order)
    {
    }

    bool operator()(ViewModel::Item *a, ViewModel::Item *b)
    {
        if (a->hidden)
            return false;
        else if (b->hidden)
            return true;
        else if (order == Qt::AscendingOrder)
            return a->lessThan(col, b);
        else
            return b->lessThan(col, a);
    }

    int col;
    Qt::SortOrder order;
};

void ViewModel::sort(int col, Qt::SortOrder order)
{
    sort_column = col;
    sort_order = order;
    Q_EMIT layoutAboutToBeChanged();
    std::stable_sort(torrents.begin(), torrents.end(), ViewModelItemCmp(col, order));
    Q_EMIT layoutChanged();
    Q_EMIT sorted();
}
}
