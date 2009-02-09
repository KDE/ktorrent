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
#include <QClipboard>
#include <QToolButton>
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
#include <KNotifyConfigWidget>
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
#include "dbus/dbus.h"
#include "pastedialog.h"
#include "ipfilterwidget.h"
#include "torrentcreatordlg.h"
#include "importdialog.h"
#include "speedlimitsdlg.h"
#include "queuemanagerwidget.h"
#include <util/timer.h>


namespace kt
{
	GUI::GUI() : core(0),pref_dlg(0)
	{
		//Marker markk("GUI::GUI()");
		core = new Core(this);
		tray_icon = new TrayIcon(core,this);
		view_man = new ViewManager(core->getGroupManager()->allGroup(),this,core);
		setupActions();
		view_man->setupActions();
		
		group_view = new GroupView(core->getGroupManager(),view_man,this);
		addToolWidget(group_view,"application-x-bittorrent",i18n("Groups"),i18n("Widget to manage torrent groups"),DOCK_LEFT);
		connect(group_view,SIGNAL(openNewTab(kt::Group*)),this,SLOT(openNewView(kt::Group*)));

		qm = new QueueManagerWidget(core->getQueueManager(),this);
		connect(core,SIGNAL(torrentAdded(bt::TorrentInterface*)),qm,SLOT(onTorrentAdded(bt::TorrentInterface*)));
		connect(core,SIGNAL(torrentRemoved(bt::TorrentInterface*)),qm,SLOT(onTorrentRemoved(bt::TorrentInterface*)));
		addToolWidget(qm,"kt-queue-manager",i18n("Queue Manager"),i18n("Widget to manage the torrent queue"),DOCK_BOTTOM);
		
		createGUI("ktorrentui.rc");
		
		status_bar = new kt::StatusBar(this);
		setStatusBar(status_bar);

		//Marker mark("core->loadTorrents()");
		core->loadTorrents();
		//mark.update();
		connect(&timer,SIGNAL(timeout()),this,SLOT(update()));
		timer.start(Settings::guiUpdateInterval());
		
		QToolButton* lc = leftCornerButton();
		QToolButton* rc = rightCornerButton();

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

		dbus_iface = new DBus(this,core,this);
		view_man->loadState(KGlobal::config());
		core->loadPlugins();
		loadState(KGlobal::config());
		notifyViewListeners(view_man->getCurrentTorrent());
		//markk.update();
		updateActions();
	}

	GUI:: ~GUI()
	{
		delete core;
	}

	void GUI::addTabPage(QWidget* page,const QString & icon,const QString & caption,const QString & tooltip,CloseTabListener* ctl)
	{
		addTab(page,caption,icon,CENTER,tooltip);
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

	void GUI::addToolWidget(QWidget* w,const QString & icon,const QString & caption,const QString & tooltip,ToolDock dock)
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
		addTab(w,caption,icon,pos,tooltip);
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
	
	bt::TorrentInterface* GUI::getCurrentTorrent()
	{
		return view_man->getCurrentTorrent();
	}
	
	void GUI::dataScan(bt::TorrentInterface* tc,bool auto_import,bool silently,const QString & dlg_caption)
	{
		ScanDlg* dlg = new ScanDlg(core,auto_import,this);
		if (!dlg_caption.isEmpty())
			dlg->setWindowTitle(dlg_caption);
		dlg->show();
		dlg->execute(tc,silently);
		core->startUpdateTimer(); // make sure update timer is running
	}

	bool GUI::selectFiles(bt::TorrentInterface* tc,bool* user,bool* start_torrent,const QString & group_hint,bool* skip_check)
	{
		FileSelectDlg dlg(core->getGroupManager(),group_hint,this);

		return dlg.execute(tc,user,start_torrent,skip_check) == QDialog::Accepted;
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
		core->load(url,QString());
	}
	
	void GUI::loadSilently(const KUrl & url)
	{
		core->loadSilently(url,QString());
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
		
		foreach (const KUrl &url,urls)
		{
			if (url.isValid())
				core->loadSilently(url,QString());
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
			foreach (const KUrl &url,urls)
			{
				if (url.isValid())
				{
					if (Settings::openMultipleTorrentsSilently())
						loadSilently(url);
					else
						load(url);
				}
			}
		}
	}
	
	void GUI::pauseQueue(bool pause)
	{
		Out(SYS_GEN|LOG_NOTICE) << "Setting paused state to " << pause << endl;
		core->setPausedState(pause);
		view_man->updateActions();
	}
	
	void GUI::onPausedStateChanged(bool paused)
	{
		queue_pause_action->setChecked(paused);
	}

	void GUI::startAllTorrentsCV()
	{
		view_man->startAllTorrents();
	}

	void GUI::stopAllTorrentsCV()
	{
		view_man->stopAllTorrents();
	}
	
	void GUI::startAllTorrents()
	{
		core->startAll(3);
	}

	void GUI::stopAllTorrents()
	{
		core->stopAll(3);
	}

	void GUI::pasteURL()
	{
		PasteDialog dlg(core, this);
		dlg.exec();
	}
	
	void GUI::paste()
	{
		if (!paste_action->isEnabled())
			return;
		
		QClipboard *cb = QApplication::clipboard();
		QString text = cb->text(QClipboard::Clipboard);
		if (text.length() == 0)
			return;
		
		KUrl url = KUrl(text);

		if (url.isValid())
			load(url);
		else
			KMessageBox::error(this,i18n("Invalid URL: %1",url.prettyUrl()));
	}

	void GUI::showPrefDialog()
	{
		if (!pref_dlg)
			pref_dlg = new PrefDialog(this,core);

		pref_dlg->updateWidgetsAndShow();
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
		new_action->setToolTip(i18n("Create a new torrent"));
		KAction* open_action = KStandardAction::open(this, SLOT(openTorrent()),ac);
		open_action->setToolTip(i18n("Open a torrent"));
		paste_action = KStandardAction::paste(this,SLOT(paste()),ac);
		KStandardAction::selectAll(view_man,SLOT(selectAll()),ac);
		
		open_silently_action = new KAction(KIcon(open_action->icon()),i18n("Open Silently"),this);
		open_silently_action->setToolTip(i18n("Open a torrent without asking any questions"));
		connect(open_silently_action,SIGNAL(triggered()),this,SLOT(openTorrentSilently()));
		ac->addAction("file_open_silently",open_silently_action);
				
		KAction* quit_action = KStandardAction::quit(kapp, SLOT(quit()), ac);
		show_status_bar_action = KStandardAction::showStatusbar(this, SLOT(showStatusBar()),ac);
		show_status_bar_action->setIcon(KIcon("kt-show-statusbar"));
		show_menu_bar_action = KStandardAction::showMenubar(this, SLOT(showMenuBar()),ac);
		KAction* pref_action = KStandardAction::preferences(this, SLOT(showPrefDialog()),ac);
		KStandardAction::keyBindings(this, SLOT(configureKeys()),ac);
		
		KStandardAction::configureToolbars(this,SLOT(configureToolBars()),ac);
		KStandardAction::configureNotifications(this,SLOT(configureNotifications()),ac);

		start_action = new KAction(KIcon("kt-start"),i18n("Start"), this);
		start_action->setToolTip(i18n("Start all selected torrents in the current tab"));
		start_action->setShortcut(KShortcut(Qt::CTRL + Qt::Key_S));
		connect(start_action,SIGNAL(triggered()),view_man,SLOT(startTorrents()));
		ac->addAction("start",start_action);

		stop_action = new KAction(KIcon("kt-stop"),i18n("Stop"),this);
		stop_action->setToolTip(i18n("Stop all selected torrents in the current tab"));
		stop_action->setShortcut(KShortcut(Qt::CTRL + Qt::Key_H));
		connect(stop_action,SIGNAL(triggered()),view_man,SLOT(stopTorrents()));
		ac->addAction("stop",stop_action);

		remove_action = new KAction(KIcon("kt-remove"),i18n("Remove"),this);
		remove_action->setToolTip(i18n("Remove all selected torrents in the current tab"));
		remove_action->setShortcut(KShortcut(Qt::Key_Delete));
		connect(remove_action,SIGNAL(triggered()),view_man,SLOT(removeTorrents()));
		ac->addAction("remove",remove_action);

		start_all_cv_action = new KAction(KIcon("kt-start-all"),i18n("Start All"),this);
		start_all_cv_action->setToolTip(i18n("Start all torrents in the current tab"));
		start_all_cv_action->setShortcut(KShortcut(Qt::SHIFT + Qt::Key_S));
		connect(start_all_cv_action,SIGNAL(triggered()),this,SLOT(startAllTorrentsCV()));
		ac->addAction("start_all",start_all_cv_action);
		
		start_all_action = new KAction(KIcon("kt-start-all"),i18n("Start All"),this);
		start_all_action->setToolTip(i18n("Start all torrents"));
		connect(start_all_action,SIGNAL(triggered()),this,SLOT(startAllTorrents()));
		
		stop_all_cv_action = new KAction(KIcon("kt-stop-all"),i18n("Stop All"),this);
		stop_all_cv_action->setToolTip(i18n("Stop all torrents in the current tab"));
		stop_all_cv_action->setShortcut(KShortcut(Qt::SHIFT + Qt::Key_H));
		connect(stop_all_cv_action,SIGNAL(triggered()),this,SLOT(stopAllTorrentsCV()));
		ac->addAction("stop_all",stop_all_cv_action);
		
		stop_all_action = new KAction(KIcon("kt-stop-all"),i18n("Stop All"),this);
		stop_all_action->setToolTip(i18n("Stop all torrents"));
		connect(stop_all_action,SIGNAL(triggered()),this,SLOT(stopAllTorrents()));
		
		paste_url_action = new KAction(KIcon(open_action->icon()),i18n("Open URL"),this);
		paste_url_action->setToolTip(i18n("Open an URL which points to a torrent"));
		paste_url_action->setShortcut(KShortcut(Qt::CTRL + Qt::Key_P));
		connect(paste_url_action,SIGNAL(triggered()),this,SLOT(pasteURL()));
		ac->addAction("paste_url",paste_url_action);

		queue_action = new KAction(KIcon("view-choose"),i18n("Enqueue/Dequeue"),this);
		queue_action->setToolTip(i18n("Enqueue or dequeue all selected torrents in the current tab"));
		queue_action->setShortcut(KShortcut(Qt::CTRL + Qt::Key_D));
		connect(queue_action,SIGNAL(triggered()),view_man,SLOT(queueTorrents()));
		ac->addAction("queue_action",queue_action);
		
		queue_pause_action = new KToggleAction(KIcon("kt-pause"),i18n("Pause KTorrent"),this);
		queue_pause_action->setToolTip(i18n("Pause all running torrents"));
		queue_pause_action->setShortcut(KShortcut(Qt::SHIFT + Qt::Key_P));
		connect(queue_pause_action,SIGNAL(toggled(bool)),this,SLOT(pauseQueue(bool)));
		queue_pause_action->setCheckedState(KGuiItem(i18n("Resume KTorrent"),"media-playback-start",
											i18n("Resume paused torrents")));
		ac->addAction("queue_pause",queue_pause_action);

		ipfilter_action = new KAction(KIcon("view-filter"),i18n("IP Filter"),this);
		ipfilter_action->setToolTip(i18n("Show the list of blocked IP addresses"));
		ipfilter_action->setShortcut(KShortcut(Qt::CTRL + Qt::Key_I));
		connect(ipfilter_action,SIGNAL(triggered()),this,SLOT(showIPFilter()));
		ac->addAction("ipfilter_action",ipfilter_action);

		data_check_action = new KAction(KIcon("kt-check-data"),i18n("Check Data"),this);
		data_check_action->setToolTip(i18n("Check all the data of a torrent"));
		data_check_action->setShortcut(KShortcut(Qt::SHIFT + Qt::Key_C));
		connect(data_check_action,SIGNAL(triggered()),view_man,SLOT(checkData()));
		ac->addAction("check_data",data_check_action);
		
		import_action = new KAction(KIcon("document-import"),i18n("Import Torrent"),this);
		import_action->setToolTip(i18n("Import a torrent"));
		import_action->setShortcut(KShortcut(Qt::SHIFT + Qt::Key_I));
		connect(import_action,SIGNAL(triggered()),this,SLOT(import()));
		ac->addAction("import",import_action);
		
#ifndef Q_WS_WIN
		import_kde3_torrents_action = new KAction(KIcon("document-import"),i18n("Import KDE3 Torrents"),this);
		import_kde3_torrents_action->setToolTip(i18n("Import all torrents from the KDE3 version of KTorrent"));
		connect(import_kde3_torrents_action,SIGNAL(triggered()),core,SLOT(importKDE3Torrents()));
		ac->addAction("import_kde3_torrents",import_kde3_torrents_action);
#else
		import_kde3_torrents_action = 0; // this action is not needed in windows
#endif
		
		speed_limits_action = new KAction(KIcon("kt-speed-limits"),i18n("Speed Limits"),this);
		speed_limits_action->setToolTip(i18n("Set the speed limits of individual torrents"));
		connect(speed_limits_action,SIGNAL(triggered()),this,SLOT(speedLimits()));
		speed_limits_action->setShortcut(KShortcut(Qt::CTRL + Qt::Key_L));
		ac->addAction("speed_limits",speed_limits_action);
		
		show_kt_action = new KAction(KIcon("kt-show-hide"),i18n("Show/Hide KTorrent"),this);
		connect(show_kt_action,SIGNAL(triggered()),this,SLOT(showOrHide()));
		show_kt_action->setGlobalShortcut(KShortcut(Qt::ALT+ Qt::CTRL + Qt::Key_T), 
										  KAction::ActiveShortcut | KAction::DefaultShortcut,KAction::Autoloading);
		show_kt_action->setShortcut(show_kt_action->globalShortcut());
		ac->addAction("show_kt",show_kt_action);

		setStandardToolBarMenuEnabled(true);
				
		QMenu* m = tray_icon->contextMenu();
		m->addAction(start_all_action);
		m->addAction(stop_all_action);
		m->addAction(queue_pause_action);
		m->addSeparator();
		m->addAction(paste_url_action);
		m->addAction(open_action);
		m->addSeparator();
		m->addAction(pref_action);
		m->addAction(show_kt_action);
		m->addSeparator();
		m->addAction(quit_action);
	}

	void GUI::update()
	{
		view_man->update();

		CurrentStats stats = core->getStats();
		status_bar->updateSpeed(stats.upload_speed,stats.download_speed);
		status_bar->updateTransfer(stats.bytes_uploaded,stats.bytes_downloaded);
		status_bar->updateDHTStatus(Globals::instance().getDHT().isRunning(),Globals::instance().getDHT().getStats());

		tray_icon->updateStats(stats);
		core->updateGuiPlugins();
		
		if (qm->isVisible())
			qm->update();
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
		newView(core->getGroupManager()->allGroup());
	}

	View* GUI::newView(kt::Group* g)
	{
		View* view = view_man->newView(core,this);
		view->setGroup(g);
		addTabPage(view,g->groupIconName(),view->caption(false),QString(),view_man);
		changeTabToolTip(view,view->caption(true));
		connect(view,SIGNAL(editingItem(bool)),this,SLOT(setPasteDisabled(bool)));
		return view;
	}
			
	void GUI::openNewView(kt::Group* g)
	{
		View* v = newView(g);
		v->setupDefaultColumns();
	}
	
	
	void GUI::openView(const QString & group_name,bool starting_up)
	{
		Group* g = core->getGroupManager()->find(group_name);
		if (!g)
		{
			g = core->getGroupManager()->findDefault(group_name);
			if (!g)
				g = core->getGroupManager()->allGroup();
		}

		View* v = newView(g);
		if (!starting_up) // if it is a new view, setup the default columns based upon the group
			v->setupDefaultColumns();
	}

	void GUI::currentTabPageChanged(QWidget* page)
	{
		view_man->onCurrentTabChanged(page);
		CloseTabListener* ctl = close_tab_map[page];
		rightCornerButton()->setEnabled(ctl != 0 && ctl->closeAllowed(page));

		notifyViewListeners(view_man->getCurrentTorrent());
		notifyCurrentTabPageListeners(page);
	}
	
	void GUI::speedLimits()
	{
		QList<bt::TorrentInterface*> sel;
		view_man->getSelection(sel);
		SpeedLimitsDlg dlg(sel.count() > 0 ? sel.front() : 0,core,this);
		dlg.exec();
	}
	
	void GUI::loadState(KSharedConfigPtr cfg)
	{
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
		
		bool hidden_on_exit = g.readEntry("hidden_on_exit",false);
		if (Settings::showSystemTrayIcon())
		{
			if (hidden_on_exit)
			{
				Out(SYS_GEN|LOG_DEBUG) << "Starting minimized" << endl;
				hide();
			}
			else
			{
				show();
			}
		}
		else
		{
			show();
		}
	}

	void GUI::saveState(KSharedConfigPtr cfg)
	{
		KConfigGroup g = cfg->group("MainWindow");
		g.writeEntry("statusbar_hidden",status_bar->isHidden());
		g.writeEntry("menubar_hidden",menuBar()->isHidden());
		g.writeEntry("hidden_on_exit",isHidden());
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
			QTimer::singleShot(500,KApplication::kApplication(),SLOT(quit()));
			return true;
		}
	}
		
	bool GUI::queryExit()
	{
		static bool first_time = true;
		if (first_time)
		{
			timer.stop();
			ideal::MainWindow::queryExit();
			hide();
			tray_icon->hide();
			core->onExit();
			first_time = false;
		}
		return true;
	}
	
	void GUI::updateActions()
	{
		view_man->updateActions();
		Uint32 nr = core->getNumTorrentsRunning();
		queue_pause_action->setEnabled(core->getPausedState() || nr > 0);
		start_all_action->setEnabled(core->getNumTorrentsNotRunning() > 0);
		stop_all_action->setEnabled(nr > 0);
	}
	
	void GUI::showOrHide()
	{
		setVisible(!isVisible());
	}

	void GUI::configureNotifications()
	{
		KNotifyConfigWidget::configure( this );
	}
	
	void GUI::setTabIcon(QWidget* tab,const QString & icon)
	{
		changeTabIcon(tab,icon);
	}
	
	void GUI::setTabText(QWidget* tab,const QString & text)
	{
		changeTabText(tab,text);
	}
	
	void GUI::setCurrentTab(QWidget* tab)
	{
		changeCurrentTab(tab);
	}
	
	QWidget* GUI::getCurrentTab()
	{
		return currentTabPage();
	}
	
	void GUI::setPasteDisabled(bool on)
	{
		paste_action->setEnabled(!on);
	}
}

#include "gui.moc"
