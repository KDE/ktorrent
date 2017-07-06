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

#include "view.h"

#include <QAction>
#include <QClipboard>
#include <QFileInfo>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QHeaderView>
#include <QMenu>
#include <QSortFilterProxyModel>

#include <KActionCollection>
#include <KFileWidget>
#include <KLocalizedString>
#include <KMessageBox>
#include <KSharedConfig>
#include <KStandardAction>
#include <KRecentDirs>
#include <KRun>

#include <interfaces/torrentinterface.h>
#include <torrent/queuemanager.h>
#include <torrent/jobqueue.h>
#include <util/functions.h>
#include <util/log.h>
#include <interfaces/functions.h>
#include <groups/group.h>
#include <groups/groupmanager.h>
#include "core.h"
#include "gui.h"
#include "viewmodel.h"
#include "dialogs/speedlimitsdlg.h"
#include "dialogs/addpeersdlg.h"
#include "interfaces/torrentactivityinterface.h"
#include "viewselectionmodel.h"
#include "viewdelegate.h"
#include "propertiesdlg.h"
#include "settings.h"
#include "viewjobtracker.h"


using namespace bt;

namespace kt
{

    View::View(Core* core, GUI* gui, QWidget* parent)
        : QTreeView(parent),
          core(core),
          gui(gui),
          group(core->getGroupManager()->allGroup()),
          num_torrents(0),
          num_running(0),
          model(nullptr)
    {
        new ViewJobTracker(this);

        model = new ViewModel(core, this);
        selection_model = new ViewSelectionModel(model, this);

        setContextMenuPolicy(Qt::CustomContextMenu);
        setRootIsDecorated(false);
        setSortingEnabled(true);
        setAlternatingRowColors(true);
        setDragEnabled(true);
        setSelectionMode(QAbstractItemView::ExtendedSelection);
        setSelectionBehavior(QAbstractItemView::SelectRows);
        setAcceptDrops(true);
        setDragDropMode(DragDrop);
        setUniformRowHeights(true);

        connect(this, &View::customContextMenuRequested, this, &View::showMenu);

        header()->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(header(), SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showHeaderMenu(const QPoint&)));
        header_menu = new QMenu(this);
        header_menu->addSection(i18n("Columns"));

        for (int i = 0; i < model->columnCount(QModelIndex()); i++)
        {
            QString col = model->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString();
            QAction * act = new QAction(col, header_menu);
            header_menu->addAction(act);
            act->setCheckable(true);
            act->setChecked(true);
            column_idx_map[act] = i;
            column_action_list.append(act);
        }

        connect(header_menu, &QMenu::triggered, this, &View::onHeaderMenuItemTriggered);

        setModel(model);
        setSelectionModel(selection_model);
        connect(selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
                this, SLOT(onCurrentItemChanged(const QModelIndex&, const QModelIndex&)));
        connect(selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection)),
                this, SLOT(onSelectionChanged(const QItemSelection&, const QItemSelection)));
        connect(model, &ViewModel::sorted, selection_model, &ViewSelectionModel::sorted);
        connect(this, &View::doubleClicked, this, &View::onDoubleClicked);

        delegate = new ViewDelegate(core, model, this);
        setItemDelegate(delegate);

        gui->setCaption(group->groupName());

        default_state = header()->saveState();

        connect(core->getGroupManager(), SIGNAL(groupAdded(Group*)), this, SLOT(onGroupAdded(Group*)));
        connect(core->getGroupManager(), SIGNAL(groupRemoved(Group*)), this, SLOT(onGroupRemoved(Group*)));
        connect(core->getGroupManager(), SIGNAL(groupRenamed(Group*)), this, SLOT(onGroupRenamed(Group*)));
    }

    View::~View()
    {
        delegate->contractAll();
    }

    void View::setupActions(KActionCollection* ac)
    {
        KStandardAction::selectAll(this, SLOT(selectAll()), ac);

        start_torrent = new QAction(QIcon::fromTheme(QStringLiteral("kt-start")), i18nc("@action Start all selected torrents in the current tab", "Start"), this);
        start_torrent->setToolTip(i18n("Start all selected torrents in the current tab"));
        connect(start_torrent, &QAction::triggered, this, &View::startTorrents);
        ac->addAction(QStringLiteral("start"), start_torrent);
        ac->setDefaultShortcut(start_torrent, QKeySequence(Qt::CTRL + Qt::Key_S));

        force_start_torrent = new QAction(QIcon::fromTheme(QStringLiteral("kt-start")), i18nc("@action Force start all selected torrents in the current tab", "Force Start"), this);
        force_start_torrent->setToolTip(i18n("Force start all selected torrents in the current tab"));
        connect(force_start_torrent, &QAction::triggered, this, &View::forceStartTorrents);
        ac->addAction(QStringLiteral("force_start"), force_start_torrent);

        stop_torrent = new QAction(QIcon::fromTheme(QStringLiteral("kt-stop")), i18nc("@action Stop all selected torrents in the current tab", "Stop"), this);
        stop_torrent->setToolTip(i18n("Stop all selected torrents in the current tab"));
        connect(stop_torrent, &QAction::triggered, this, &View::stopTorrents);
        ac->addAction(QStringLiteral("stop"), stop_torrent);
        ac->setDefaultShortcut(stop_torrent, QKeySequence(Qt::CTRL + Qt::Key_H));

        pause_torrent = new QAction(QIcon::fromTheme(QStringLiteral("media-playback-pause")), i18nc("@action Pause all selected torrents in the current tab", "Pause"), this);
        pause_torrent->setToolTip(i18n("Pause all selected torrents in the current tab"));
        connect(pause_torrent, &QAction::triggered, this, &View::pauseTorrents);
        ac->addAction(QStringLiteral("pause"), pause_torrent);

        remove_torrent = new QAction(QIcon::fromTheme(QStringLiteral("kt-remove")), i18nc("@action Remove all selected torrents in the current tab", "Remove"), this);
        remove_torrent->setToolTip(i18n("Remove all selected torrents in the current tab"));
        connect(remove_torrent, &QAction::triggered, this, &View::removeTorrents);
        ac->addAction(QStringLiteral("remove"), remove_torrent);
        ac->setDefaultShortcut(remove_torrent, QKeySequence(Qt::SHIFT + Qt::Key_Delete));

        start_all = new QAction(QIcon::fromTheme(QStringLiteral("kt-start-all")), i18nc("@action Start all torrents in the current tab", "Start All"), this);
        start_all->setToolTip(i18n("Start all torrents in the current tab"));
        connect(start_all, &QAction::triggered, this, &View::startAllTorrents);
        ac->addAction(QStringLiteral("start_all"), start_all);
        ac->setDefaultShortcut(start_all, QKeySequence(Qt::SHIFT + Qt::Key_S));

        stop_all = new QAction(QIcon::fromTheme(QStringLiteral("kt-stop-all")), i18nc("@action Stop all torrents in the current tab", "Stop All"), this);
        stop_all->setToolTip(i18n("Stop all torrents in the current tab"));
        connect(stop_all, &QAction::triggered, this, &View::stopAllTorrents);
        ac->addAction(QStringLiteral("stop_all"), stop_all);
        ac->setDefaultShortcut(stop_all, QKeySequence(Qt::SHIFT + Qt::Key_H));

        remove_torrent_and_data = new QAction(QIcon::fromTheme(QStringLiteral("kt-remove")), i18n("Remove Torrent and Data"), this);
        connect(remove_torrent_and_data, &QAction::triggered, this, &View::removeTorrentsAndData);
        ac->addAction(QStringLiteral("view_remove_torrent_and_data"), remove_torrent_and_data);
        ac->setDefaultShortcut(remove_torrent_and_data, QKeySequence(Qt::CTRL + Qt::Key_Delete));

        rename_torrent = new QAction(i18n("Rename Torrent"), this);
        connect(rename_torrent, &QAction::triggered, this, &View::renameTorrent);
        ac->addAction(QStringLiteral("view_rename_torrent"), rename_torrent);

        add_peers = new QAction(QIcon::fromTheme(QStringLiteral("list-add")), i18n("Add Peers"), this);
        connect(add_peers, &QAction::triggered, this, &View::addPeers);
        ac->addAction(QStringLiteral("view_add_peers"), add_peers);

        manual_announce = new QAction(i18n("Manual Announce"), this);
        connect(manual_announce, &QAction::triggered, this, &View::manualAnnounce);
        ac->addAction(QStringLiteral("view_announce"), manual_announce);
        ac->setDefaultShortcut(manual_announce, QKeySequence(Qt::SHIFT + Qt::Key_A));

        do_scrape = new QAction(i18n("Scrape"), this);
        connect(do_scrape, &QAction::triggered, this, &View::scrape);
        ac->addAction(QStringLiteral("view_scrape"), do_scrape);

        preview = new QAction(QIcon::fromTheme(QStringLiteral("document-open")), i18n("Preview"), this);
        connect(preview, &QAction::triggered, this, &View::previewTorrents);
        ac->addAction(QStringLiteral("view_preview"), preview);

        data_dir = new QAction(QIcon::fromTheme(QStringLiteral("folder-open")), i18n("Data Directory"), this);
        connect(data_dir, &QAction::triggered, this, &View::openDataDir);
        ac->addAction(QStringLiteral("view_open_data_dir"), data_dir);

        tor_dir = new QAction(QIcon::fromTheme(QStringLiteral("folder-open")), i18n("Temporary Directory"), this);
        connect(tor_dir, &QAction::triggered, this, &View::openTorDir);
        ac->addAction(QStringLiteral("view_open_tmp_dir"), tor_dir);

        move_data = new QAction(i18n("Move Data"), this);
        connect(move_data, &QAction::triggered, this, &View::moveData);
        ac->addAction(QStringLiteral("view_move_data"), move_data);

        torrent_properties = new QAction(i18n("Settings"), this);
        connect(torrent_properties, &QAction::triggered, this, &View::showProperties);
        ac->addAction(QStringLiteral("view_torrent_properties"), torrent_properties);

        remove_from_group = new QAction(i18n("Remove from Group"), this);
        connect(remove_from_group, &QAction::triggered, this, &View::removeFromGroup);
        ac->addAction(QStringLiteral("view_remove_from_group"), remove_from_group);

        add_to_new_group = new QAction(QIcon::fromTheme(QStringLiteral("document-new")), i18n("New Group"), this);
        connect(add_to_new_group, &QAction::triggered, this, &View::addToNewGroup);
        ac->addAction(QStringLiteral("view_add_to_new_group"), add_to_new_group);

        check_data = new QAction(QIcon::fromTheme(QStringLiteral("kt-check-data")), i18n("Check Data"), this);
        check_data->setToolTip(i18n("Check all the data of a torrent"));
        connect(check_data, &QAction::triggered, this, &View::checkData);
        ac->addAction(QStringLiteral("check_data"), check_data);
        ac->setDefaultShortcut(check_data, QKeySequence(Qt::SHIFT + Qt::Key_C));

        GroupManager* gman = core->getGroupManager();
        for (GroupManager::Itr i = gman->begin(); i != gman->end(); i++)
        {
            if (!i->second->isStandardGroup())
            {
                QAction * act = new QAction(QIcon::fromTheme(QStringLiteral("application-x-bittorrent")), i->first, this);
                connect(act, &QAction::triggered, this, &View::addToGroupItemTriggered);
                group_actions.insert(i->second, act);
            }
        }

        open_dir_menu = new QAction(QIcon::fromTheme(QStringLiteral("folder-open")), i18n("Open Directory"), this);
        ac->addAction(QStringLiteral("OpenDirMenu"), open_dir_menu);

        groups_menu = new QAction(QIcon::fromTheme(QStringLiteral("application-x-bittorrent")), i18n("Add to Group"), this);
        ac->addAction(QStringLiteral("GroupsSubMenu"), groups_menu);

        copy_url = new QAction(QIcon::fromTheme(QStringLiteral("edit-copy")), i18n("Copy Torrent URL"), this);
        connect(copy_url, &QAction::triggered, this, &View::copyTorrentURL);
        ac->addAction(QStringLiteral("view_copy_url"), copy_url);

        export_torrent = new QAction(QIcon::fromTheme(QStringLiteral("document-export")), i18n("Export Torrent"), this);
        connect(export_torrent, &QAction::triggered, this, &View::exportTorrent);
        ac->addAction(QStringLiteral("view_export_torrent"), export_torrent);

        speed_limits = new QAction(QIcon::fromTheme(QStringLiteral("kt-speed-limits")), i18n("Speed Limits"), this);
        speed_limits->setToolTip(i18n("Set the speed limits of individual torrents"));
        connect(speed_limits, &QAction::triggered, this, &View::speedLimits);
        ac->addAction(QStringLiteral("speed_limits"), speed_limits);
        ac->setDefaultShortcut(speed_limits, QKeySequence(Qt::CTRL + Qt::Key_L));
    }

    struct StartAndStopAllVisitor
    {
        QAction* start_all;
        QAction* stop_all;

        StartAndStopAllVisitor(QAction* start_all, QAction* stop_all) : start_all(start_all), stop_all(stop_all)
        {}

        bool operator()(bt::TorrentInterface* tc)
        {
            if (tc->getJobQueue()->runningJobs())
                return true;

            const TorrentStats& s = tc->getStats();
            if (s.running || (tc->isAllowedToStart() && !tc->overMaxRatio() && !tc->overMaxSeedTime()))
                stop_all->setEnabled(true);
            else
                start_all->setEnabled(true);

            return !stop_all->isEnabled() || !start_all->isEnabled();
        }
    };

    void View::updateActions()
    {
        QList<bt::TorrentInterface*> sel;
        getSelection(sel);

        bool qm_enabled = !Settings::manuallyControlTorrents();
        bool en_start = false;
        bool en_stop = false;
        bool en_remove = false;
        bool en_prev = false;
        bool en_announce = false;
        bool en_add_peer = false;
        bool en_pause = false;

        for (bt::TorrentInterface* tc : qAsConst(sel))
        {
            const TorrentStats& s = tc->getStats();

            if (tc->readyForPreview() && !s.multi_file_torrent)
                en_prev = true;

            if (tc->getJobQueue()->runningJobs())
                continue;

            en_remove = true;
            if (!s.running)
            {
                if (qm_enabled)
                {
                    // Queued torrents can be stopped, and not started
                    if (s.queued)
                        en_stop = true;
                    else
                        en_start = true;
                }
                else
                {
                    en_start = true;
                }
            }
            else
            {
                en_stop = true;
                if (tc->announceAllowed())
                    en_announce = true;

                if (!s.paused)
                    en_pause = true;
                else
                    en_start = true;
            }

            if (!s.priv_torrent)
            {
                en_add_peer = true;
            }
        }

        en_add_peer = en_add_peer && en_stop;

        start_torrent->setEnabled(en_start);
        force_start_torrent->setEnabled(en_start);
        stop_torrent->setEnabled(en_stop);
        remove_torrent->setEnabled(en_remove);
        remove_torrent_and_data->setEnabled(en_remove);
        pause_torrent->setEnabled(en_pause);
        preview->setEnabled(en_prev);
        add_peers->setEnabled(en_add_peer);
        manual_announce->setEnabled(en_announce);
        do_scrape->setEnabled(sel.count() > 0);
        move_data->setEnabled(sel.count() > 0);

        remove_from_group->setEnabled(group && !group->isStandardGroup());
        groups_menu->setEnabled(group_actions.count() > 0);
        check_data->setEnabled(sel.count() > 0);

        rename_torrent->setEnabled(sel.count() == 1);
        data_dir->setEnabled(sel.count() == 1);
        tor_dir->setEnabled(sel.count() == 1);
        open_dir_menu->setEnabled(sel.count() == 1);
        add_to_new_group->setEnabled(sel.count() > 0);
        copy_url->setEnabled(sel.count() == 1 && sel.front()->loadUrl().isValid());
        export_torrent->setEnabled(sel.count() == 1);

        if (qm_enabled)
        {
            start_all->setEnabled(false);
            stop_all->setEnabled(false);
            StartAndStopAllVisitor v(start_all, stop_all);
            model->visit(v);
        }
        else
        {
            start_all->setEnabled(numRunningTorrents() < numTorrents());
            stop_all->setEnabled(numRunningTorrents() > 0);
        }
    }

    void View::setGroup(Group* g)
    {
        group = g;
        model->setGroup(group);
        update();
        selectionModel()->clear();
        gui->setCaption(g->groupName());
    }

    void View::update()
    {
        if (!uniformRowHeights() && !delegate->hasExtenders())
            setUniformRowHeights(true);

        if (!model->update(delegate))
        {
            // model wasn't resorted, so update individual items
            const QModelIndexList& to_update = model->updateList();
            for (const QModelIndex& idx : to_update)
                QAbstractItemView::update(idx);
        }
    }

    void View::startTorrents()
    {
        QList<bt::TorrentInterface*> sel;
        getSelection(sel);
        if (sel.count() > 0)
            core->start(sel);
    }

    void View::forceStartTorrents()
    {
        QList<bt::TorrentInterface*> sel;
        getSelection(sel);
        if (sel.count() == 0)
            return;

        QueueManager* qm = core->getQueueManager();
        if (qm->enabled())
        {
            // Give everybody in the selection a high priority
            int prio = qm->count();
            int idx = 0;
            for (bt::TorrentInterface* tc : qAsConst(sel))
                tc->setPriority(prio + sel.count() - idx++);

            core->start(sel);
        }
        else
            core->start(sel);
    }


    void View::stopTorrents()
    {
        QList<bt::TorrentInterface*> sel;
        getSelection(sel);
        if (sel.count() > 0)
            core->stop(sel);
    }

    void View::pauseTorrents()
    {
        QList<bt::TorrentInterface*> sel;
        getSelection(sel);
        if (sel.count() > 0)
            core->pause(sel);
    }

    void View::removeTorrents()
    {
        QList<bt::TorrentInterface*> sel;
        getSelection(sel);
        for (bt::TorrentInterface* tc : qAsConst(sel))
        {
            if (tc && !tc->getJobQueue()->runningJobs())
            {
                const TorrentStats& s = tc->getStats();
                bool data_to = false;
                if (!s.completed)
                {
                    QString msg = i18n("The torrent <b>%1</b> has not finished downloading, "
                                       "do you want to delete the incomplete data, too?", tc->getDisplayName());
                    int ret = KMessageBox::questionYesNoCancel(
                                  this,
                                  msg,
                                  i18n("Remove Download"),
                                  KGuiItem(i18n("Delete Data")),
                                  KGuiItem(i18n("Keep Data")),
                                  KStandardGuiItem::cancel());
                    if (ret == KMessageBox::Cancel)
                        return;
                    else if (ret == KMessageBox::Yes)
                        data_to = true;
                }
                core->remove(tc, data_to);
            }
        }
    }

    void View::removeTorrentsAndData()
    {
        QList<bt::TorrentInterface*> sel;
        getSelection(sel);
        if (sel.count() == 0)
            return;

        QStringList names;
        for (bt::TorrentInterface* tc : qAsConst(sel))
        {
            names.append(tc->getDisplayName());
        }

        QString msg = i18n("You will lose all the downloaded data of the following torrents. Are you sure you want to do this?");
        if (KMessageBox::warningYesNoList(this, msg, names, i18n("Remove Torrent"),
                                          KStandardGuiItem::remove(), KStandardGuiItem::cancel()) == KMessageBox::Yes)
        {
            core->remove(sel, true);
        }
    }

    void View::startAllTorrents()
    {
        QList<bt::TorrentInterface*> all;
        model->allTorrents(all);
        core->start(all);
    }

    void View::stopAllTorrents()
    {
        QList<bt::TorrentInterface*> all;
        model->allTorrents(all);
        core->stop(all);
    }

    void View::addPeers()
    {
        QList<bt::TorrentInterface*> sel;
        getSelection(sel);
        if (sel.count() > 0)
        {
            AddPeersDlg dlg(sel[0], this);
            dlg.exec();
        }
    }

    void View::manualAnnounce()
    {
        QList<bt::TorrentInterface*> sel;
        getSelection(sel);
        for (bt::TorrentInterface* tc : qAsConst(sel))
        {
            if (tc->getStats().running)
                tc->updateTracker();
        }
    }

    void View::scrape()
    {
        QList<bt::TorrentInterface*> sel;
        getSelection(sel);
        for (bt::TorrentInterface* tc : qAsConst(sel))
        {
            tc->scrapeTracker();
        }
    }

    void View::previewTorrents()
    {
        QList<bt::TorrentInterface*> sel;
        getSelection(sel);
        for (bt::TorrentInterface* tc : qAsConst(sel))
        {
            if (tc->readyForPreview() && !tc->getStats().multi_file_torrent)
            {
                new KRun(QUrl::fromLocalFile(tc->getStats().output_path), 0, true);
            }
        }
    }

    void View::openDataDir()
    {
        QList<bt::TorrentInterface*> sel;
        getSelection(sel);
        for (bt::TorrentInterface* tc : qAsConst(sel))
        {
            if (tc->getStats().multi_file_torrent)
                new KRun(QUrl::fromLocalFile(tc->getStats().output_path), 0, true);
            else
                new KRun(QUrl::fromLocalFile(tc->getDataDir()), 0, true);
        }
    }

    void View::openTorDir()
    {
        QList<bt::TorrentInterface*> sel;
        getSelection(sel);
        for (bt::TorrentInterface* tc : qAsConst(sel))
        {
            new KRun(QUrl::fromLocalFile(tc->getTorDir()), 0, true);
        }
    }

    void View::moveData()
    {
        QList<bt::TorrentInterface*> sel;
        getSelection(sel);
        if (sel.count() == 0)
            return;

        QString recentDirClass;
        QString dir = QFileDialog::getExistingDirectory(this, i18n("Select a directory to move the data to."),
                                                        KFileWidget::getStartUrl(QUrl(QStringLiteral("kfiledialog:///saveTorrentData")), recentDirClass).toLocalFile());

        if (dir.isEmpty())
            return;

        if (!recentDirClass.isEmpty())
            KRecentDirs::add(recentDirClass, dir);

        for (bt::TorrentInterface* tc : qAsConst(sel))
        {
            if (core->checkMissingFiles(tc))
                tc->changeOutputDir(dir, bt::TorrentInterface::MOVE_FILES);
        }
    }

    void View::showProperties()
    {
        QList<bt::TorrentInterface*> sel;
        getSelection(sel);
        if (sel.count() == 0)
            return;

        PropertiesDlg dlg(sel.front(), this);
        dlg.exec();
    }

    void View::removeFromGroup()
    {
        if (!group || group->isStandardGroup())
            return;

        QList<bt::TorrentInterface*> sel;
        getSelection(sel);
        for (bt::TorrentInterface* tc : qAsConst(sel))
            group->removeTorrent(tc);
        core->getGroupManager()->saveGroups();
        update();
        num_running++; // set these wrong so that the caption is updated on the next update
        num_torrents++;
    }

    void View::renameTorrent()
    {
        QModelIndexList indices = selectionModel()->selectedRows();
        if (indices.count() == 0)
            return;

        QModelIndex idx = indices.front();
        QTreeView::edit(model->index(idx.row(), 0));
        editingItem(true);
    }

    void View::checkData()
    {
        QList<bt::TorrentInterface*> sel;
        getSelection(sel);
        for (bt::TorrentInterface* tc : qAsConst(sel))
        {
            if (tc->getStats().status != bt::ALLOCATING_DISKSPACE)
                core->doDataCheck(tc);
        }

        core->startUpdateTimer(); // make sure update timer of core is running
    }

    void View::showMenu(const QPoint& pos)
    {
        QMenu* view_menu = gui->getTorrentActivity()->part()->menu(QStringLiteral("ViewMenu"));
        if (!view_menu)
            return;

        gui->getTorrentActivity()->part()->plugActionList(QStringLiteral("view_groups_list"), group_actions.values());
        gui->getTorrentActivity()->part()->unplugActionList(QStringLiteral("view_columns_list"));
        gui->getTorrentActivity()->part()->plugActionList(QStringLiteral("view_columns_list"), column_action_list);
        view_menu->popup(viewport()->mapToGlobal(pos));
    }

    void View::showHeaderMenu(const QPoint& pos)
    {
        header_menu->popup(header()->mapToGlobal(pos));
    }

    void View::getSelection(QList<bt::TorrentInterface*> & sel)
    {
        QModelIndexList indices = selectionModel()->selectedRows();
        model->torrentsFromIndexList(indices, sel);
    }

    void View::restoreState(const QByteArray& state)
    {
        if (!state.isEmpty())
        {
            QHeaderView* v = header();
            v->restoreState(state);
            sortByColumn(v->sortIndicatorSection(), v->sortIndicatorOrder());
            model->sort(v->sortIndicatorSection(), v->sortIndicatorOrder());
        }

        QMap<QAction*, int>::iterator i = column_idx_map.begin();
        while (i != column_idx_map.end())
        {
            QAction* act = i.key();
            bool hidden = header()->isSectionHidden(i.value());
            act->setChecked(!hidden);
            if (!hidden && header()->sectionSize(i.value()) == 0)
                header()->resizeSection(i.value(), 20);
            i++;
        }
    }


    bt::TorrentInterface* View::getCurrentTorrent()
    {
        return model->torrentFromIndex(selectionModel()->currentIndex());
    }

    void View::onCurrentItemChanged(const QModelIndex& current, const QModelIndex& /*previous*/)
    {
        //Out(SYS_GEN|LOG_DEBUG) << "onCurrentItemChanged " << current.row() << endl;
        bt::TorrentInterface* tc = model->torrentFromIndex(current);
        currentTorrentChanged(tc);
    }

    void View::onHeaderMenuItemTriggered(QAction* act)
    {
        int idx = column_idx_map[act];
        if (act->isChecked())
            header()->showSection(idx);
        else
            header()->hideSection(idx);
    }

    void View::onSelectionChanged(const QItemSelection& /*selected*/, const QItemSelection& /*deselected*/)
    {
        updateActions();
        torrentSelectionChanged();
    }

    void View::closeEditor(QWidget* editor, QAbstractItemDelegate::EndEditHint hint)
    {
        QTreeView::closeEditor(editor, hint);
        editingItem(false);
        setFocus();
    }

    bool View::edit(const QModelIndex& index, EditTrigger trigger, QEvent* event)
    {
        bool ret = QTreeView::edit(index, trigger, event);
        if (ret)
            editingItem(true);

        return ret;
    }

    void View::onDoubleClicked(const QModelIndex& index)
    {
        if (index.column() == 0) // double clicking on column 0 will change the name of a torrent
            return;

        bt::TorrentInterface* tc = model->torrentFromIndex(index);
        if (tc)
        {
            if (tc->getStats().multi_file_torrent)
                new KRun(QUrl::fromLocalFile(tc->getStats().output_path), 0, true);
            else
                new KRun(QUrl::fromLocalFile(tc->getDataDir()), 0, true);
        }
    }

    void View::extend(TorrentInterface* tc, kt::Extender* widget, bool close_similar)
    {
        setUniformRowHeights(false);
        delegate->extend(tc, widget, close_similar);
        if (group && !group->isMember(tc))
            delegate->hideExtender(tc);
    }

    void View::onCurrentGroupChanged(Group* g)
    {
        setGroup(g);
    }

    void View::onGroupAdded(Group* g)
    {
        gui->getTorrentActivity()->part()->unplugActionList(QStringLiteral("view_groups_list"));
        QAction * act = new QAction(QIcon::fromTheme(QStringLiteral("application-x-bittorrent")), g->groupName(), this);
        connect(act, &QAction::triggered, this, &View::addToGroupItemTriggered);
        group_actions.insert(g, act);
        gui->getTorrentActivity()->part()->plugActionList(QStringLiteral("view_groups_list"), group_actions.values());
    }

    void View::onGroupRemoved(Group* g)
    {
        if (group == g)
            setGroup(core->getGroupManager()->allGroup());

        gui->getTorrentActivity()->part()->unplugActionList(QStringLiteral("view_groups_list"));
        QMap<Group*, QAction*>::iterator i = group_actions.find(g);
        if (i != group_actions.end())
        {
            delete i.value();
            group_actions.erase(i);
        }

        gui->getTorrentActivity()->part()->plugActionList(QStringLiteral("view_groups_list"), group_actions.values());
    }

    void View::onGroupRenamed(Group* g)
    {
        QMap<Group*, QAction*>::iterator j = group_actions.find(g);
        if (j != group_actions.end())
            j.value()->setText(g->groupName());
    }

    void View::addToGroupItemTriggered()
    {
        QAction * s = (QAction*)sender();
        Group* g = 0;
        QMap<Group*, QAction*>::iterator j = group_actions.begin();
        while (j != group_actions.end() && !g)
        {
            if (j.value() == s)
                g = j.key();
            j++;
        }

        if (!g)
            return;

        QList<bt::TorrentInterface*> sel;
        getSelection(sel);
        for (bt::TorrentInterface* tc : qAsConst(sel))
        {
            g->addTorrent(tc, false);
        }
        core->getGroupManager()->saveGroups();
    }

    void View::addToNewGroup()
    {
        Group* g = gui->getTorrentActivity()->addNewGroup();
        if (g)
        {
            QList<bt::TorrentInterface*> sel;
            getSelection(sel);
            for (bt::TorrentInterface* tc : qAsConst(sel))
            {
                g->addTorrent(tc, false);
            }
            core->getGroupManager()->saveGroups();
        }
    }

    void View::copyTorrentURL()
    {
        QList<bt::TorrentInterface*> sel;
        getSelection(sel);
        if (sel.count() == 0)
            return;

        bt::TorrentInterface* tc = sel.front();
        if (!tc->loadUrl().isValid())
            return;

        QClipboard* cb = QApplication::clipboard();
        cb->setText(tc->loadUrl().toDisplayString());
    }

    void View::speedLimits()
    {
        QList<bt::TorrentInterface*> sel;
        getSelection(sel);
        SpeedLimitsDlg dlg(sel.count() > 0 ? sel.front() : 0, core, gui->getMainWindow());
        dlg.exec();
    }

    void View::exportTorrent()
    {
        QList<bt::TorrentInterface*> sel;
        getSelection(sel);
        if (sel.count() == 1)
        {
            bt::TorrentInterface* tc = sel.front();

            QString recentDirClass;
            QString fn = QFileDialog::getSaveFileName(gui, QString(),
                                                    KFileWidget::getStartUrl(QUrl(QLatin1String("kfiledialog:///exportTorrent")), recentDirClass).toLocalFile(),
                                                    kt::TorrentFileFilter(false));

            if (fn.isEmpty())
                return;

            if (!recentDirClass.isEmpty())
                KRecentDirs::add(recentDirClass, QFileInfo(fn).absolutePath());


            KIO::file_copy(QUrl::fromLocalFile(tc->getTorDir() + QLatin1String("torrent")), QUrl::fromLocalFile(fn), -1, KIO::Overwrite);
        }
    }

    void View::setFilterString(const QString& filter)
    {
        model->setFilterString(filter);
        model->update(delegate);
    }

    void View::keyPressEvent(QKeyEvent* event)
    {
        if (event->key() == Qt::Key_Delete)
        {
            removeTorrents();
            event->accept();
        }
        else
            QTreeView::keyPressEvent(event);
    }

}

