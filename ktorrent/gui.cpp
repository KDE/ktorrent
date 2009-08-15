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
#include <plugin/pluginmanager.h>
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
#include "queuemanagerwidget.h"
#include <util/timer.h>
#include <gui/activitybar.h>
#include "torrentactivity.h"
#include <gui/centralwidget.h>


namespace kt
{
	GUI::GUI() : core(0),pref_dlg(0)
	{
		//Marker markk("GUI::GUI()");
		core = new Core(this);
		tray_icon = new TrayIcon(core,this);
		setupActions();
		
		central = new CentralWidget(this);
		setCentralWidget(central);
		torrent_activity = new TorrentActivity(core,this,0);
		addActivity(torrent_activity);
		
		createGUI("ktorrentui.rc");
		
		status_bar = new kt::StatusBar(this);
		setStatusBar(status_bar);

		//Marker mark("core->loadTorrents()");
		core->loadTorrents();
		//mark.update();
		connect(&timer,SIGNAL(timeout()),this,SLOT(update()));
		timer.start(Settings::guiUpdateInterval());

		applySettings();
		connect(core,SIGNAL(settingsChanged()),this,SLOT(applySettings()));

		if (Settings::showSystemTrayIcon())
		{
			tray_icon->updateMaxRateMenus();
			tray_icon->show();
		}
		else
			tray_icon->hide();		

		dbus_iface = new DBus(this,core,this);
		core->loadPlugins();
		loadState(KGlobal::config());
		
		//markk.update();
		updateActions();
		core->startUpdateTimer();
	}

	GUI:: ~GUI()
	{
		delete core;
	}
	
	void GUI::addActivity(Activity* act)
	{
		central->activityBar()->addActivity(act);
	}
	
	void GUI::removeActivity(Activity* act)
	{
		central->activityBar()->removeActivity(act);
	}
	
	void GUI::setCurrentActivity(Activity* act)
	{
		central->activityBar()->setCurrentActivity(act);
	}

	void GUI::addPrefPage(PrefPageInterface* page)
	{
		if (!pref_dlg)
		{
			pref_dlg = new PrefDialog(this,core);
			pref_dlg->loadState(KGlobal::config());
		}

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
	
	
	void GUI::dataScanStarted(ScanListener* listener)
	{
		torrent_activity->dataScanStarted(listener);
		core->startUpdateTimer(); // make sure update timer is running
	}
	
	void GUI::dataScanClosed(ScanListener* listener)
	{
		torrent_activity->dataScanClosed(listener);
	}


	bool GUI::selectFiles(bt::TorrentInterface* tc,bool* start_torrent,const QString & group_hint,bool* skip_check)
	{
		FileSelectDlg dlg(core->getGroupManager(),group_hint,this);
		dlg.loadState(KGlobal::config());
		bool ret = dlg.execute(tc,start_torrent,skip_check) == QDialog::Accepted;
		dlg.saveState(KGlobal::config());
		return ret;
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
		torrent_activity->updateActions();
	}
	
	void GUI::onPausedStateChanged(bool paused)
	{
		queue_pause_action->setChecked(paused);
	}
	
	void GUI::startAllTorrents()
	{
		core->startAll();
	}

	void GUI::stopAllTorrents()
	{
		core->stopAll();
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
		
		start_all_action = new KAction(KIcon("kt-start-all"),i18n("Start All"),this);
		start_all_action->setToolTip(i18n("Start all torrents"));
		connect(start_all_action,SIGNAL(triggered()),this,SLOT(startAllTorrents()));
		
		stop_all_action = new KAction(KIcon("kt-stop-all"),i18n("Stop All"),this);
		stop_all_action->setToolTip(i18n("Stop all torrents"));
		connect(stop_all_action,SIGNAL(triggered()),this,SLOT(stopAllTorrents()));
		
		paste_url_action = new KAction(KIcon(open_action->icon()),i18n("Open URL"),this);
		paste_url_action->setToolTip(i18n("Open a URL which points to a torrent"));
		paste_url_action->setShortcut(KShortcut(Qt::CTRL + Qt::Key_P));
		connect(paste_url_action,SIGNAL(triggered()),this,SLOT(pasteURL()));
		ac->addAction("paste_url",paste_url_action);
		
		queue_pause_action = new KToggleAction(KIcon("kt-pause"),i18n("Pause KTorrent"),this);
		ac->addAction("queue_pause",queue_pause_action);
		queue_pause_action->setToolTip(i18n("Pause all running torrents"));
		queue_pause_action->setShortcut(KShortcut(Qt::SHIFT + Qt::Key_P));
		queue_pause_action->setGlobalShortcut(KShortcut(Qt::ALT + Qt::SHIFT + Qt::Key_P));
		connect(queue_pause_action,SIGNAL(toggled(bool)),this,SLOT(pauseQueue(bool)));
		queue_pause_action->setCheckedState(KGuiItem(i18n("Resume KTorrent"),"media-playback-start",
											i18n("Resume paused torrents")));
		
		ipfilter_action = new KAction(KIcon("view-filter"),i18n("IP Filter"),this);
		ipfilter_action->setToolTip(i18n("Show the list of blocked IP addresses"));
		ipfilter_action->setShortcut(KShortcut(Qt::CTRL + Qt::Key_I));
		connect(ipfilter_action,SIGNAL(triggered()),this,SLOT(showIPFilter()));
		ac->addAction("ipfilter_action",ipfilter_action);

		
		
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
		
		show_kt_action = new KAction(KIcon("kt-show-hide"),i18n("Show/Hide KTorrent"),this);
		connect(show_kt_action,SIGNAL(triggered()),this,SLOT(showOrHide()));
		ac->addAction("show_kt",show_kt_action);
		show_kt_action->setGlobalShortcut(KShortcut(Qt::ALT+ Qt::SHIFT + Qt::Key_T), 
										  KAction::ActiveShortcut | KAction::DefaultShortcut,KAction::Autoloading);
		
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
		CurrentStats stats = core->getStats();
		status_bar->updateSpeed(stats.upload_speed,stats.download_speed);
		status_bar->updateTransfer(stats.bytes_uploaded,stats.bytes_downloaded);
		status_bar->updateDHTStatus(Globals::instance().getDHT().isRunning(),Globals::instance().getDHT().getStats());

		tray_icon->updateStats(stats);
		core->updateGuiPlugins();
		torrent_activity->update();
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
	
	void GUI::loadState(KSharedConfigPtr cfg)
	{
		setAutoSaveSettings("MainWindow",true);
		central->loadState(cfg);
		torrent_activity->loadState(cfg);
		
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
		saveMainWindowSettings(g);
		g.writeEntry("statusbar_hidden",status_bar->isHidden());
		g.writeEntry("menubar_hidden",menuBar()->isHidden());
		g.writeEntry("hidden_on_exit",isHidden());
		torrent_activity->saveState(cfg);
		central->saveState(cfg);
		if (pref_dlg)
			pref_dlg->saveState(cfg);
		cfg->sync();
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
			saveState(KGlobal::config());
			timer.stop();
			ScanDlg::cancelAllScans();
			hide();
			tray_icon->hide();
			core->onExit();
			first_time = false;
		}
		return true;
	}
	
	void GUI::updateActions()
	{
		torrent_activity->updateActions();
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
	
	void GUI::setPasteDisabled(bool on)
	{
		paste_action->setEnabled(!on);
	}
	
	QWidget* GUI::container(const QString & name)
	{
		return guiFactory()->container(name, this);
	}
	
	TorrentActivityInterface* GUI::getTorrentActivity()
	{
		return torrent_activity;
	}
}

#include "gui.moc"
