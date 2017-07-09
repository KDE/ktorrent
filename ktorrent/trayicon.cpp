/***************************************************************************
 *   Copyright (C) 2005-2007 by                                            *
 *   Joris Guisson <joris.guisson@gmail.com>                               *
 *   Ivan Vasic <ivasic@gmail.com>                                         *
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

#include "trayicon.h"

#include <algorithm>

#include <QPainter>
#include <QIcon>
#include <QLocale>
#include <QToolTip>

#include <KActionCollection>
#include <KLocalizedString>
#include <KNotification>
#include <KPassivePopup>

#include <util/functions.h>
#include <net/socketmonitor.h>
#include <util/log.h>
#include <settings.h>
#include <interfaces/torrentactivityinterface.h>
#include <torrent/queuemanager.h>
#include "core.h"
#include "gui.h"


using namespace bt;

namespace kt
{


    TrayIcon::TrayIcon(Core* core, GUI* parent) : QObject(parent)
        , core(core)
        , mwnd(parent)
        , previousDownloadHeight(0)
        , previousUploadHeight(0)
        , max_upload_rate(nullptr)
        , max_download_rate(nullptr)
        , status_notifier_item(nullptr)
        , queue_suspended(false)
        , menu(nullptr)
    {
        connect(core, &Core::openedSilently, this, &TrayIcon::torrentSilentlyOpened);
        connect(core, &Core::finished, this, &TrayIcon::finished);
        connect(core, &Core::torrentStoppedByError, this, &TrayIcon::torrentStoppedByError);
        connect(core, &Core::maxShareRatioReached, this, &TrayIcon::maxShareRatioReached);
        connect(core, &Core::maxSeedTimeReached, this, &TrayIcon::maxSeedTimeReached);
        connect(core, &Core::corruptedData, this, &TrayIcon::corruptedData);
        connect(core, &Core::queuingNotPossible, this, &TrayIcon::queuingNotPossible);
        connect(core, &Core::canNotStart, this, &TrayIcon::canNotStart);
        connect(core, &Core::lowDiskSpace, this, &TrayIcon::lowDiskSpace);
        connect(core, &Core::canNotLoadSilently, this, &TrayIcon::cannotLoadTorrentSilently);
        connect(core, &Core::dhtNotEnabled, this, &TrayIcon::dhtNotEnabled);
        connect(core->getQueueManager(), SIGNAL(suspendStateChanged(bool)),
                this, SLOT(suspendStateChanged(bool)));

        suspendStateChanged(core->getQueueManager()->getSuspendedState());
    }

    TrayIcon::~TrayIcon()
    {}

    void TrayIcon::hide()
    {
        if (!status_notifier_item)
            return;

        delete status_notifier_item;
        status_notifier_item = nullptr;
        menu = nullptr;
        max_download_rate = max_upload_rate = nullptr;
    }

    void TrayIcon::show()
    {
        if (status_notifier_item)
        {
            suspendStateChanged(core->getQueueManager()->getSuspendedState());
            return;
        }

        status_notifier_item = new KStatusNotifierItem(mwnd);
        connect(status_notifier_item, &KStatusNotifierItem::secondaryActivateRequested, this, &TrayIcon::secondaryActivate);

        menu = status_notifier_item->contextMenu();

        max_upload_rate = new SetMaxRate(core, SetMaxRate::UPLOAD, menu);
        max_upload_rate->setTitle(i18n("Set max upload speed"));
        max_download_rate = new SetMaxRate(core, SetMaxRate::DOWNLOAD, menu);
        max_download_rate->setTitle(i18n("Set max download speed"));
        menu->addMenu(max_download_rate);
        menu->addMenu(max_upload_rate);
        menu->addSeparator();

        KActionCollection* ac = mwnd->getTorrentActivity()->part()->actionCollection();
        menu->addAction(ac->action(QStringLiteral("start_all")));
        menu->addAction(ac->action(QStringLiteral("stop_all")));
        menu->addAction(ac->action(QStringLiteral("queue_suspend")));
        menu->addSeparator();

        ac = mwnd->actionCollection();
        menu->addAction(ac->action(QStringLiteral("paste_url")));
        menu->addAction(ac->action(QString::fromUtf8(KStandardAction::name(KStandardAction::Open))));
        menu->addSeparator();
        menu->addAction(ac->action(QString::fromUtf8(KStandardAction::name(KStandardAction::Preferences))));
        menu->addSeparator();


        status_notifier_item->setIconByName(QStringLiteral("ktorrent"));
        status_notifier_item->setCategory(KStatusNotifierItem::ApplicationStatus);
        status_notifier_item->setStatus(KStatusNotifierItem::Passive);
        status_notifier_item->setStandardActionsEnabled(true);
        status_notifier_item->setContextMenu(menu);

        queue_suspended = core->getQueueManager()->getSuspendedState();
        if (queue_suspended)
            status_notifier_item->setOverlayIconByName(QStringLiteral("kt-pause"));
    }


    void TrayIcon::updateStats(const CurrentStats& stats)
    {
        if (!status_notifier_item)
            return;

        status_notifier_item->setStatus(core->getQueueManager()->getNumRunning(QueueManager::DOWNLOADS) > 0 ?
                                        KStatusNotifierItem::Active : KStatusNotifierItem::Passive);
        QString tip = i18n("Download speed: <b>%1</b><br/>"
                           "Upload speed: <b>%2</b><br/>"
                           "Received: <b>%3</b><br/>"
                           "Transmitted: <b>%4</b>",
                           BytesPerSecToString((double)stats.download_speed),
                           BytesPerSecToString((double)stats.upload_speed),
                           BytesToString(stats.bytes_downloaded),
                           BytesToString(stats.bytes_uploaded));

        status_notifier_item->setToolTip(QStringLiteral("ktorrent"), i18n("Status"), tip);
    }

    void TrayIcon::showPassivePopup(const QString& msg, const QString& title)
    {
        if (status_notifier_item)
            status_notifier_item->showMessage(title, msg, QStringLiteral("ktorrent"));
    }

    void TrayIcon::cannotLoadTorrentSilently(const QString& msg)
    {
        if (!Settings::showPopups())
            return;

        KNotification::event(QStringLiteral("CannotLoadSilently"), msg, QPixmap(), mwnd);
    }

    void TrayIcon::dhtNotEnabled(const QString& msg)
    {
        if (!Settings::showPopups())
            return;

        KNotification::event(QStringLiteral("DHTNotEnabled"), msg, QPixmap(), mwnd);
    }

    void TrayIcon::torrentSilentlyOpened(bt::TorrentInterface* tc)
    {
        if (!Settings::showPopups())
            return;

        QString msg = i18n("<b>%1</b> was silently opened.",
                           tc->getDisplayName());
        KNotification::event(QStringLiteral("TorrentSilentlyOpened"), msg, QPixmap(), mwnd);
    }

    void TrayIcon::finished(bt::TorrentInterface* tc)
    {
        if (!Settings::showPopups())
            return;

        const TorrentStats& s = tc->getStats();
        double speed_up = (double)s.bytes_uploaded;
        double speed_down = (double)(s.bytes_downloaded - s.imported_bytes);

        QString msg = i18n("<b>%1</b> has completed downloading."
                           "<br>Average speed: %2 DL / %3 UL.",
                           tc->getDisplayName(),
                           BytesPerSecToString(speed_down / tc->getRunningTimeDL()),
                           BytesPerSecToString(speed_up / tc->getRunningTimeUL()));

        KNotification::event(QStringLiteral("TorrentFinished"), msg, QPixmap(), mwnd);
    }

    void TrayIcon::maxShareRatioReached(bt::TorrentInterface* tc)
    {
        if (!Settings::showPopups())
            return;

        const TorrentStats& s = tc->getStats();
        double speed_up = (double)s.bytes_uploaded;

        QString msg = i18n("<b>%1</b> has reached its maximum share ratio of %2 and has been stopped."
                           "<br>Uploaded %3 at an average speed of %4.",
                           tc->getDisplayName(),
                           QLocale().toString(s.max_share_ratio, 'g', 2),
                           BytesToString(s.bytes_uploaded),
                           BytesPerSecToString(speed_up / tc->getRunningTimeUL()));

        KNotification::event(QStringLiteral("MaxShareRatioReached"), msg, QPixmap(), mwnd);
    }

    void TrayIcon::maxSeedTimeReached(bt::TorrentInterface* tc)
    {
        if (!Settings::showPopups())
            return;

        const TorrentStats& s = tc->getStats();
        double speed_up = (double)s.bytes_uploaded;

        QString msg = i18n("<b>%1</b> has reached its maximum seed time of %2 hours and has been stopped."
                           "<br>Uploaded %3 at an average speed of %4.",
                           tc->getDisplayName(),
                           QLocale().toString(s.max_seed_time, 'g', 2),
                           BytesToString(s.bytes_uploaded),
                           BytesPerSecToString(speed_up / tc->getRunningTimeUL()));

        KNotification::event(QStringLiteral("MaxSeedTimeReached"), msg, QPixmap(), mwnd);
    }

    void TrayIcon::torrentStoppedByError(bt::TorrentInterface* tc, QString msg)
    {
        if (!Settings::showPopups())
            return;

        QString err_msg = i18n("<b>%1</b> has been stopped with the following error: <br>%2",
                               tc->getDisplayName(), msg);

        KNotification::event(QStringLiteral("TorrentStoppedByError"), err_msg, QPixmap(), mwnd);
    }

    void TrayIcon::corruptedData(bt::TorrentInterface* tc)
    {
        if (!Settings::showPopups())
            return;

        QString err_msg = i18n("Corrupted data has been found in the torrent <b>%1</b>"
                               "<br>It would be a good idea to do a data integrity check on the torrent.", tc->getDisplayName());

        KNotification::event(QStringLiteral("CorruptedData"), err_msg, QPixmap(), mwnd);
    }

    void TrayIcon::queuingNotPossible(bt::TorrentInterface* tc)
    {
        if (!Settings::showPopups())
            return;

        const TorrentStats& s = tc->getStats();

        QString msg;

        if (tc->overMaxRatio())
            msg = i18n("<b>%1</b> has reached its maximum share ratio of %2 and cannot be enqueued. "
                       "<br>Remove the limit manually if you want to continue seeding.",
                       tc->getDisplayName(), QLocale().toString(s.max_share_ratio, 'g', 2));
        else
            msg = i18n("<b>%1</b> has reached its maximum seed time of %2 hours and cannot be enqueued. "
                       "<br>Remove the limit manually if you want to continue seeding.",
                       tc->getDisplayName(), QLocale().toString(s.max_seed_time, 'g', 2));

        KNotification::event(QStringLiteral("QueueNotPossible"), msg, QPixmap(), mwnd);
    }

    void TrayIcon::canNotStart(bt::TorrentInterface* tc, bt::TorrentStartResponse reason)
    {
        if (!Settings::showPopups())
            return;

        QString msg = i18n("Cannot start <b>%1</b>: <br>", tc->getDisplayName());
        switch (reason)
        {
        case bt::QM_LIMITS_REACHED:
            if (tc->getStats().bytes_left_to_download == 0)
            {
                // is a seeder
                msg += i18np("Cannot seed more than 1 torrent. <br>",
                             "Cannot seed more than %1 torrents. <br>", Settings::maxSeeds());
            }
            else
            {
                msg += i18np("Cannot download more than 1 torrent. <br>",
                             "Cannot download more than %1 torrents. <br>", Settings::maxDownloads());
            }
            msg += i18n("Go to Settings -> Configure KTorrent, if you want to change the limits.");
            KNotification::event(QStringLiteral("CannotStart"), msg, QPixmap(), mwnd);
            break;
        case bt::NOT_ENOUGH_DISKSPACE:
            msg += i18n("There is not enough diskspace available.");
            KNotification::event(QStringLiteral("CannotStart"), msg, QPixmap(), mwnd);
            break;
        default:
            break;
        }
    }

    void TrayIcon::lowDiskSpace(bt::TorrentInterface* tc, bool stopped)
    {
        if (!Settings::showPopups())
            return;

        QString msg = i18n("Your disk is running out of space.<br /><b>%1</b> is being downloaded to '%2'.", tc->getDisplayName(), tc->getDataDir());

        if (stopped)
            msg.prepend(i18n("Torrent has been stopped.<br />"));

        KNotification::event(QStringLiteral("LowDiskSpace"), msg);
    }

    void TrayIcon::updateMaxRateMenus()
    {
        if (max_download_rate && max_upload_rate)
        {
            max_upload_rate->update();
            max_download_rate->update();
        }
    }



    SetMaxRate::SetMaxRate(Core* core, Type t, QWidget* parent) : QMenu(parent)
    {
        setIcon(t == UPLOAD ? QIcon::fromTheme(QStringLiteral("kt-set-max-upload-speed")) : QIcon::fromTheme(QStringLiteral("kt-set-max-download-speed")));
        m_core = core;
        type = t;
        makeMenu();
        connect(this, &SetMaxRate::triggered, this, &SetMaxRate::onTriggered);
        connect(this, &SetMaxRate::aboutToShow, this, &SetMaxRate::update);
    }

    SetMaxRate::~SetMaxRate()
    {}

    void SetMaxRate::makeMenu()
    {
        int rate = (type == UPLOAD) ? net::SocketMonitor::getUploadCap() / 1024 : net::SocketMonitor::getDownloadCap() / 1024;
        int maxBandwidth = (rate > 0) ? rate : (type == UPLOAD) ? 0 : 20 ;
        int delta = 0;
        int maxBandwidthRounded;

        setTitle(i18n("Speed limit in KiB/s"));

        unlimited = addAction(i18n("Unlimited"));
        unlimited->setCheckable(true);
        unlimited->setChecked(rate == 0);

        if ((maxBandwidth % 5) >= 3)
            maxBandwidthRounded = maxBandwidth + 5 - (maxBandwidth % 5);
        else
            maxBandwidthRounded = maxBandwidth - (maxBandwidth % 5);

        QList<int> values;
        for (int i = 0; i < 15; i++)
        {
            if (delta == 0)
                values.append(maxBandwidth);
            else
            {
                if ((maxBandwidth % 5) != 0)
                {
                    values.append(maxBandwidthRounded - delta);
                    values.append(maxBandwidthRounded + delta);
                }
                else
                {
                    values.append(maxBandwidth - delta);
                    values.append(maxBandwidth + delta);
                }
            }

            delta += (delta >= 50) ? 50 : (delta >= 20) ? 10 : 5;
        }

        std::sort(values.begin(), values.end());
        for (int v : qAsConst(values))
        {
            if (v >= 1)
            {
                QAction* act = addAction(QString::number(v));
                act->setCheckable(true);
                act->setChecked(rate == v);
            }
        }
    }

    void SetMaxRate::update()
    {
        clear();
        makeMenu();
    }

    void SetMaxRate::onTriggered(QAction* act)
    {
        int rate;
        if (act == unlimited)
            rate = 0;
        else
            rate = act->text().remove(QLatin1Char('&')).toInt(); // remove ampersands

        if (type == UPLOAD)
        {
            Settings::setMaxUploadRate(rate);
            net::SocketMonitor::setUploadCap(Settings::maxUploadRate() * 1024);
        }
        else
        {
            Settings::setMaxDownloadRate(rate);
            net::SocketMonitor::setDownloadCap(Settings::maxDownloadRate() * 1024);
        }
        Settings::self()->save();
    }

    void TrayIcon::suspendStateChanged(bool suspended)
    {
        queue_suspended = suspended;
        if (status_notifier_item)
            status_notifier_item->setOverlayIconByName(suspended?QStringLiteral("kt-pause"):QString());
    }

    void TrayIcon::secondaryActivate(const QPoint& pos)
    {
        Q_UNUSED(pos);
        core->setSuspendedState(!core->getSuspendedState());
    }

}
