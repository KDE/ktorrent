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
#include <qtimer.h>

#include <kconfig.h>
#include <klocale.h>
#include <kaction.h>
#include <kmenu.h>
#include <kmenubar.h>
#include <ktoggleaction.h>
#include <kactioncollection.h>
#include <kmessagebox.h>
#include <kstatusbar.h>
#include <kapplication.h>
#include <kshortcutsdialog.h>
#include <kedittoolbar.h>
#include <kstandardaction.h>
#include <kfiledialog.h>
#include <kpushbutton.h>
#include <kxmlguifactory.h>
#include <kio/jobclasses.h>
#include <kio/jobuidelegate.h>

#include <interfaces/torrentinterface.h>
#include <torrent/queuemanager.h>
#include <torrent/torrentcontrol.h>
#include <util/log.h>
#include <dht/dhtbase.h>
#include <groups/group.h>
#include <groups/groupmanager.h>
#include <pluginmanager.h>
#include <settings.h>
#include "gui.h"
#include "core.h"
#include "view.h"
#include "viewmanager.h"
#include "fileselectdlg.h"
#include "prefdialog.h"
#include "statusbar.h"
#include "groupview.h"
#include "scandlg.h"
#include "trayicon.h"
#include "dbus.h"
#include "pastedialog.h"
#include "ipfilterwidget.h"
#include "torrentcreatordlg.h"
#include "importdialog.h"
#include "speedlimitsdlg.h"
#include "queuemanagerwidget.h"

namespace kt
{
	GUI::GUI() : core(0),pref_dlg(0)
	{
		core = new Core(this);
		tray_icon = new TrayIcon(core,this);
		setupActions();
		setActionsEnabled((ActionEnableFlags)0);
		view_man = new ViewManager(core->getGroupManager()->allGroup(),this,core);
		connect(view_man,SIGNAL(enableActions(ActionEnableFlags)),this,SLOT(setActionsEnabled(ActionEnableFlags)));
		
		group_view = new GroupView(core->getGroupManager(),view_man,actionCollection(),this);
		addToolWidget(group_view,"view-choose",i18n("Groups"),DOCK_LEFT);
		connect(group_view,SIGNAL(openNewTab(kt::Group*)),this,SLOT(openView(kt::Group*)));

		qm = new QueueManagerWidget(core->getQueueManager(),this);
		connect(core,SIGNAL(torrentAdded(bt::TorrentInterface*)),qm,SLOT(onTorrentAdded(bt::TorrentInterface*)));
		connect(core,SIGNAL(torrentRemoved(bt::TorrentInterface*)),qm,SLOT(onTorrentRemoved(bt::TorrentInterface*)));
		addToolWidget(qm,"ktqueuemanager",i18n("Queue Manager"),DOCK_BOTTOM);
		
		status_bar = new kt::StatusBar(this);
		setStatusBar(status_bar);

		core->loadTorrents();
		connect(&timer,SIGNAL(timeout()),this,SLOT(update()));
		timer.start(Settings::guiUpdateInterval());
		
		KPushButton* lc = leftCornerButton();
		KPushButton* rc = rightCornerButton();

		lc->setIcon(KIcon("tab-new"));
		connect(lc,SIGNAL(clicked()),this,SLOT(newView()));
		rc->setIcon(KIcon("tab-close"));
		connect(rc,SIGNAL(clicked()),this,SLOT(closeTab()));

		applySettings();
		connect(core,SIGNAL(settingsChanged()),this,SLOT(applySettings()));

		currentTabPageChanged(currentTabPage());

		if (Settings::showSystemTrayIcon())
		{
			tray_icon->updateMaxRateMenus();
			tray_icon->show();
		}
		else
			tray_icon->hide();		

		dbus_iface = new DBus(this,core);
		
		core->loadPlugins();
		loadState(KGlobal::config());
		notifyViewListeners(view_man->getCurrentTorrent());
	}

	GUI:: ~GUI()
	{
	}

	void GUI::addTabPage(QWidget* page,const QString & icon,const QString & caption,CloseTabListener* ctl)
	{
		addTab(page,caption,icon);
		close_tab_map[page] = ctl;	
		currentTabPageChanged(currentTabPage());
	}

	void GUI::removeTabPage(QWidget* page)
	{
		removeTab(page);
		close_tab_map.remove(page);
		currentTabPageChanged(currentTabPage());
	}

	void GUI::addPrefPage(PrefPageInterface* page)
	{
		if (!pref_dlg)
			pref_dlg = new PrefDialog(this,core);

		pref_dlg->addPrefPage(page);
	}

	void GUI::removePrefPage(PrefPageInterface* page)
	{
		if (pref_dlg)
			pref_dlg->removePrefPage(page);
	}

	StatusBarInterface* GUI::getStatusBar()
	{
		return status_bar;
	}

	void GUI::mergePluginGui(Plugin* p)
	{
		guiFactory()->addClient(p);
	}

	void GUI::removePluginGui(Plugin* p)
	{
		guiFactory()->removeClient(p);
	}

	void GUI::addToolWidget(QWidget* w,const QString & icon,const QString & caption,ToolDock dock)
	{
		ideal::MainWindow::TabPosition pos = LEFT;
		switch (dock)
		{
			case DOCK_BOTTOM:
				pos = BOTTOM;
				break;
			case DOCK_LEFT:
				pos = LEFT;
				break;
			case DOCK_RIGHT:
				pos = RIGHT;
				break;
		}
		addTab(w,caption,icon,pos);
	}
	
	void GUI::removeToolWidget(QWidget* w)
	{
		removeTab(w,LEFT);
		removeTab(w,RIGHT);
		removeTab(w,BOTTOM);
	}

	const TorrentInterface* GUI::getCurrentTorrent() const
	{
		return view_man->getCurrentTorrent();
	}
	
	void GUI::dataScan(bt::TorrentInterface* tc,bool auto_import,bool silently,const QString & dlg_caption)
	{
		ScanDlg* dlg = new ScanDlg(core,auto_import,this);
		dlg->show();
		dlg->execute(tc,silently);
		core->startUpdateTimer(); // make sure update timer is running
	}

	bool GUI::selectFiles(bt::TorrentInterface* tc,bool* user,bool* start_torrent)
	{
		FileSelectDlg dlg(core->getGroupManager(),this);

		return dlg.execute(tc,user,start_torrent) == QDialog::Accepted;
	}

	void GUI::errorMsg(const QString & err)
	{
		KMessageBox::error(this,err);
	}

	void GUI::errorMsg(KIO::Job* j)
	{
		if (j->error())
			j->ui()->showErrorMessage();
	}

	void GUI::infoMsg(const QString & info)
	{
		KMessageBox::information(this,info);
	}
	
	
	void GUI::load(const KUrl & url)
	{
		core->load(url);
	}
	
	void GUI::loadSilently(const KUrl & url)
	{
		core->loadSilently(url);
	}

	void GUI::createTorrent()
	{
		TorrentCreatorDlg dlg(core,this,this);
		dlg.exec();
	}
	
	void GUI::openTorrentSilently()
	{
		QString filter = "*.torrent|" + i18n("Torrent Files") + "\n*|" + i18n("All Files");
		KUrl::List urls = KFileDialog::getOpenUrls(KUrl("kfiledialog:///openTorrent"), filter, this, i18n("Open Location"));

		if (urls.count() == 0)
			return;
		
		foreach (KUrl url,urls)
		{
			if (url.isValid())
				core->loadSilently(url);
		}
	}

	void GUI::openTorrent()
	{
		QString filter = "*.torrent|" + i18n("Torrent Files") + "\n*|" + i18n("All Files");
		KUrl::List urls = KFileDialog::getOpenUrls(KUrl("kfiledialog:///openTorrent"), filter, this, i18n("Open Location"));

		if (urls.count() == 0)
			return;
		else if (urls.count() == 1)
		{
			KUrl url = urls.front();
			if (url.isValid())
				load(url);
		}
		else 
		{
			// load multiple torrents silently
			foreach (KUrl url,urls)
			{
				if (url.isValid())
					core->loadSilently(url);
			}
		}
	}

	void GUI::startTorrent()
	{
		view_man->startTorrents();
	}

	void GUI::stopTorrent()
	{
		view_man->stopTorrents();
	}

	void GUI::removeTorrent()
	{
		view_man->removeTorrents();
	}

	void GUI::queueTorrent()
	{
		view_man->queueTorrents();
	}
	
	void GUI::pauseQueue(bool pause)
	{
		Out(SYS_GEN|LOG_DEBUG) << "pauseQUeue " << pause << endl;
		core->setPausedState(pause);
	}
	
	void GUI::onPausedStateChanged(bool paused)
	{
		queue_pause_action->setChecked(paused);
	}

	void GUI::startAllTorrents()
	{
		view_man->startAllTorrents();
	}

	void GUI::stopAllTorrents()
	{
		view_man->stopAllTorrents();
	}

	void GUI::checkData()
	{
		view_man->checkData();
	}

	void GUI::pasteURL()
	{
		PasteDialog dlg(core, this);
		dlg.exec();
	}

	void GUI::showPrefDialog()
	{
		if (!pref_dlg)
			pref_dlg = new PrefDialog(this,core);

		pref_dlg->show();
	}

	void GUI::showStatusBar()
	{
		 if (show_status_bar_action->isChecked()) 
			status_bar->show();
		 else
			status_bar->hide();
	}

	void GUI::showMenuBar()
	{
		if (show_menu_bar_action->isChecked())
			menuBar()->show();
		else
			menuBar()->hide();
	}

	void GUI::showIPFilter()
	{
		IPFilterWidget dlg(this);
		dlg.exec();
	}

	void GUI::configureKeys()
	{
		KShortcutsDialog::configure(actionCollection());
	}

	void GUI::configureToolBars()
	{
		saveMainWindowSettings( KGlobal::config()->group("MainWindow"));
		KEditToolBar dlg(factory());
		connect(&dlg,SIGNAL(newToolBarConfig()),this,SLOT(newToolBarConfig()));
		dlg.exec();
	}

	void GUI::newToolBarConfig() // This is called when OK, Apply or Defaults is clicked
	{
		applyMainWindowSettings( KGlobal::config()->group("MainWindow") );
	}
	
	void GUI::import()
	{
		ImportDialog dlg(core,this);
		dlg.exec();
	}

	void GUI::setupActions()
	{
		KActionCollection* ac = actionCollection();
		KAction* new_action = KStandardAction::openNew(this,SLOT(createTorrent()),ac);
		KAction* open_action = KStandardAction::open(this, SLOT(openTorrent()),ac);
		
		open_silently_action = new KAction(KIcon(open_action->icon()),i18n("Open Silently"),this);
		connect(open_silently_action,SIGNAL(triggered()),this,SLOT(openTorrentSilently()));
		ac->addAction("file_open_silently",open_silently_action);
				
		KAction* quit_action = KStandardAction::quit(kapp, SLOT(quit()), ac);
		show_status_bar_action = KStandardAction::showStatusbar(this, SLOT(showStatusBar()),ac);
		show_menu_bar_action = KStandardAction::showMenubar(this, SLOT(showMenuBar()),ac);
		KAction* pref_action = KStandardAction::preferences(this, SLOT(showPrefDialog()),ac);
		KStandardAction::keyBindings(this, SLOT(configureKeys()),ac);
		
		KStandardAction::configureToolbars(this,SLOT(configureToolBars()),ac);

		start_action = new KAction(KIcon("ktstart"),i18n("Start"), this);
		connect(start_action,SIGNAL(triggered()),this,SLOT(startTorrent()));
		ac->addAction("start",start_action);

		stop_action = new KAction(KIcon("ktstop"),i18n("Stop"),this);
		connect(stop_action,SIGNAL(triggered()),this,SLOT(stopTorrent()));
		ac->addAction("stop",stop_action);

		remove_action = new KAction(KIcon("ktremove"),i18n("Remove"),this);
		connect(remove_action,SIGNAL(triggered()),this,SLOT(removeTorrent()));
		ac->addAction("remove",remove_action);

		start_all_action = new KAction(KIcon("ktstart_all"),i18n("Start All"),this);
		connect(start_all_action,SIGNAL(triggered()),this,SLOT(startAllTorrents()));
		ac->addAction("start_all",start_all_action);

		stop_all_action = new KAction(KIcon("ktstop_all"),i18n("Stop All"),this);
		connect(stop_all_action,SIGNAL(triggered()),this,SLOT(stopAllTorrents()));
		ac->addAction("stop_all",stop_all_action);
		
		paste_url_action = new KAction(KIcon("edit-paste"),i18n("Paste Torrent URL"),this);
		connect(paste_url_action,SIGNAL(triggered()),this,SLOT(pasteURL()));
		ac->addAction("paste_url",paste_url_action);

		queue_action = new KAction(KIcon("view-choose"),i18n("Enqueue/Dequeue"),this);
		connect(queue_action,SIGNAL(triggered()),this,SLOT(queueTorrent()));
		ac->addAction("queue_action",queue_action);
		
		queue_pause_action = new KToggleAction(KIcon("media-playback-pause"),i18n("Pause"),this);
		queue_pause_action->setToolTip(i18n("Pause all running torrents"));
		connect(queue_pause_action,SIGNAL(toggled(bool)),this,SLOT(pauseQueue(bool)));
		queue_pause_action->setCheckedState(KGuiItem(i18n("Resume"),"media-playback-start",i18n("Resume paused torrents")));
		ac->addAction("queue_pause",queue_pause_action);

		ipfilter_action = new KAction(KIcon("view-filter"),i18n("IP Filter"),this);
		connect(ipfilter_action,SIGNAL(triggered()),this,SLOT(showIPFilter()));
		ac->addAction("ipfilter_action",ipfilter_action);

		data_check_action = new KAction(i18n("Check Data"),this);
		connect(data_check_action,SIGNAL(triggered()),this,SLOT(checkData()));
		ac->addAction("check_data",data_check_action);
		
		import_action = new KAction(KIcon("document-import"),i18n("Import Torrent"),this);
		connect(import_action,SIGNAL(triggered()),this,SLOT(import()));
		ac->addAction("import",import_action);
		
		speed_limits_action = new KAction(i18n("Speed Limits"),this);
		connect(speed_limits_action,SIGNAL(triggered()),this,SLOT(speedLimits()));
		ac->addAction("speed_limits",speed_limits_action);
		
		show_kt_action = new KAction(i18n("Show/Hide KTorrent"),this);
		connect(show_kt_action,SIGNAL(triggered()),this,SLOT(showOrHide()));
		show_kt_action->setGlobalShortcut(KShortcut(Qt::ALT+ Qt::CTRL + Qt::Key_T), 
										  KAction::ActiveShortcut | KAction::DefaultShortcut,KAction::Autoloading);
		show_kt_action->setShortcut(show_kt_action->globalShortcut());
		ac->addAction("show_kt",show_kt_action);

				
		QMenu* m = tray_icon->contextMenu();
		m->addAction(start_all_action);
		m->addAction(stop_all_action);
		m->addSeparator();
		m->addAction(paste_url_action);
		m->addAction(open_action);
		m->addAction(new_action);
		m->addSeparator();
		m->addAction(pref_action);
		m->addAction(show_kt_action);
		m->addSeparator();
		m->addAction(quit_action);
		createGUI("ktorrentui.rc");
	}

	void GUI::update()
	{
		view_man->update();

		CurrentStats stats = core->getStats();
		status_bar->updateSpeed(stats.upload_speed,stats.download_speed);
		status_bar->updateTransfer(stats.bytes_uploaded,stats.bytes_downloaded);
		status_bar->updateDHTStatus(Globals::instance().getDHT().isRunning(),Globals::instance().getDHT().getStats());

		tray_icon->updateStats(stats,Settings::showSpeedBarInTrayIcon(),Settings::downloadBandwidth(), Settings::uploadBandwidth());
		core->updateGuiPlugins();
	}

	void GUI::applySettings()
	{
		//Apply GUI update interval
		timer.setInterval(Settings::guiUpdateInterval());
		if (Settings::showSystemTrayIcon())
		{
			tray_icon->updateMaxRateMenus();
			tray_icon->show();
		}
		else
			tray_icon->hide();
	}

	void GUI::closeTab()
	{
		QWidget* w = currentTabPage();
		if (!w || !close_tab_map.contains(w))
			return;

		CloseTabListener* ctl = close_tab_map[w];
		if (ctl)
			ctl->tabCloseRequest(this,w);
	}

	void GUI::newView()
	{
		openView(core->getGroupManager()->allGroup());
	}

	void GUI::openView(kt::Group* g)
	{
		View* view = view_man->newView(core,this);
		view->setGroup(g);
		addTabPage(view,g->groupIconName(),view->caption(),view_man);
	}
	
	void GUI::openView(const QString & group_name)
	{
		Group* g = core->getGroupManager()->find(group_name);
		if (!g)
		{
			g = core->getGroupManager()->findDefault(group_name);
			if (!g)
				g = core->getGroupManager()->allGroup();
		}

		openView(g);
	}

	void GUI::currentTabPageChanged(QWidget* page)
	{
		view_man->onCurrentTabChanged(page);
		CloseTabListener* ctl = close_tab_map[page];
		rightCornerButton()->setEnabled(ctl != 0 && ctl->closeAllowed(page));

		notifyViewListeners(view_man->getCurrentTorrent());
	}
	
	void GUI::speedLimits()
	{
		SpeedLimitsDlg dlg(core,this);
		dlg.exec();
	}
	
	void GUI::loadState(KSharedConfigPtr cfg)
	{
		view_man->loadState(cfg);
		group_view->loadState(cfg);
		qm->loadState(cfg);
		ideal::MainWindow::loadState(cfg);
		
		KConfigGroup g = cfg->group("MainWindow");
		bool statusbar_hidden = g.readEntry("statusbar_hidden",false);
		status_bar->setHidden(statusbar_hidden);
		show_status_bar_action->setChecked(!statusbar_hidden);

		bool menubar_hidden = g.readEntry("menubar_hidden",false);
		menuBar()->setHidden(menubar_hidden);
		show_menu_bar_action->setChecked(!menubar_hidden);
	}

	void GUI::saveState(KSharedConfigPtr cfg)
	{
		KConfigGroup g = cfg->group("MainWindow");
		g.writeEntry("statusbar_hidden",status_bar->isHidden());
		g.writeEntry("menubar_hidden",menuBar()->isHidden());
		view_man->saveState(cfg);
		group_view->saveState(cfg);
		qm->saveState(cfg);
		ideal::MainWindow::saveState(cfg);
	}

	void GUI::currentTorrentChanged(bt::TorrentInterface* tc)
	{
		notifyViewListeners(tc);
	}

	bool GUI::queryClose()
	{
		if (Settings::showSystemTrayIcon() && !KApplication::kApplication()->sessionSaving())
		{
			hide();
			return false;
		}
		else
		{
			return true;
		}
	}
		
	bool GUI::queryExit()
	{
		timer.stop();
		ideal::MainWindow::queryExit();
		hide();
		tray_icon->hide();
		core->onExit();
		return true;
	}

	void GUI::setActionsEnabled(ActionEnableFlags flags)
	{
		start_action->setEnabled(flags & START);
		stop_action->setEnabled(flags & STOP);
		remove_action->setEnabled(flags & REMOVE);
		start_all_action->setEnabled(flags & START_ALL);
		stop_all_action->setEnabled(flags & STOP_ALL);
		queue_pause_action->setEnabled(core->getPausedState() || flags & STOP_ALL);
	}
	
	void GUI::showOrHide()
	{
		setVisible(!isVisible());
	}
}

#include "gui.moc"
