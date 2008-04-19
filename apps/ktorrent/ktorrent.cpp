/***************************************************************************
 *   Copyright (C) 2005 by                                                 *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/



#include <qdragobject.h>
#include <qsplitter.h>
#include <qvbox.h>
#include <qlabel.h>
#include <qtooltip.h>
#include <qtoolbutton.h>

#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kiconloader.h>
#include <kdeversion.h>
#include <kmenubar.h>
#include <kstatusbar.h>
#include <kkeydialog.h>
#include <kaccel.h>
#include <kstandarddirs.h>
#include <kio/netaccess.h>
#include <kfiledialog.h>
#include <kconfig.h>
#include <kurl.h>
#include <kurldrag.h>
#include <kprogress.h>
#include <kpopupmenu.h>
#include <ktabwidget.h>
#include <kedittoolbar.h>
#include <ksqueezedtextlabel.h>
#include <kpushbutton.h>

#include <kstdaccel.h>
#include <kaction.h>
#include <kstdaction.h>

#include <interfaces/torrentinterface.h>
#include <torrent/peermanager.h>
#include <torrent/chunkmanager.h>
#include <torrent/uploadcap.h>
#include <torrent/downloadcap.h>
#include <util/error.h>
#include <torrent/serverauthenticate.h>
#include <torrent/globals.h>
#include <torrent/tracker.h>
#include <torrent/downloader.h>
#include <torrent/choker.h>
#include <torrent/server.h>
#include <torrent/downloadcap.h>
#include <torrent/udptrackersocket.h>
#include <net/socketmonitor.h>
#include <util/log.h>
#include <util/fileops.h>
#include <kademlia/dhtbase.h>

#include "ktorrentcore.h"
#include "ktorrentview.h"
#include "ktorrent.h"
#include "pref.h"
#include "settings.h"
#include "trayicon.h"
#include "ktorrentdcop.h"
#include "torrentcreatordlg.h"
#include "pastedialog.h"
#include "queuedialog.h"
#include "ipfilterwidget.h"
#include <util/functions.h>
#include <interfaces/functions.h>
#include <interfaces/plugin.h>
#include <interfaces/prefpageinterface.h>
#include <newui/dtabwidget.h>
#include <pluginmanager.h>
#include <groups/group.h>
#include <groups/groupview.h>
#include <groups/groupmanager.h>
#include <mse/streamsocket.h>
#include "viewmanager.h"
#include "ktorrentviewitem.h"



namespace kt
{
	QString DataDir();
}

#include <util/profiler.h>



using namespace bt;
using namespace kt;

KTorrent::KTorrent()
	: DMainWindow(0,"KTorrent"),m_group_view(0),
		m_view_man(0), m_systray_icon(0),m_status_prog(0)
{
	setHidden(true);
	//setToolviewStyle(KMdi::TextAndIcon);
	connect(this,SIGNAL(widgetChanged(QWidget*)),this,SLOT(currentTabChanged(QWidget*)));
	
	m_view_man = new ViewManager(this);
	m_group_view = new kt::GroupView(m_view_man,actionCollection());
	connect(m_group_view,SIGNAL(openNewTab(kt::Group*)),this,SLOT(openView(kt::Group*)));
	

	m_pref = new KTorrentPreferences(*this);
	
	m_core = new KTorrentCore(this);
	m_core->setGroupManager(m_group_view->groupManager());
	
	m_systray_icon = new TrayIcon(m_core, this);
	
	m_group_view->loadGroups();
	m_view_man->restoreViewState(KGlobal::config(),this);
	
	QToolButton* tb = new QToolButton(m_activeTabWidget);
	tb->setIconSet(SmallIcon("tab_new"));
	tb->adjustSize();
	connect(tb,SIGNAL(clicked()),this,SLOT(openDefaultView()));
	m_activeTabWidget->setCornerWidget(tb, TopLeft);
	
	connect(m_group_view,SIGNAL(currentGroupChanged( kt::Group* )),
			this,SLOT(groupChanged(kt::Group*)));
	
	connect(m_group_view,SIGNAL(groupRenamed(kt::Group*)),
			this,SLOT(groupRenamed(kt::Group*)));
	
	connect(m_group_view,SIGNAL(groupRemoved(kt::Group*)),
			this,SLOT(groupRemoved(kt::Group*)));
	
	connect(m_core,SIGNAL(torrentAdded(kt::TorrentInterface* )),
			m_view_man,SLOT(addTorrent(kt::TorrentInterface* )));

	connect(m_core,SIGNAL(torrentRemoved(kt::TorrentInterface* )),
			m_view_man,SLOT(removeTorrent(kt::TorrentInterface* )));

	connect(m_core, SIGNAL(torrentRemoved( kt::TorrentInterface* )),
			m_group_view, SLOT(onTorrentRemoved( kt::TorrentInterface* )));
	
	m_statusInfo = new KSqueezedTextLabel(this);
	m_statusSpeed = new QLabel(this);
	m_statusTransfer = new QLabel(this);
	m_statusDHT = new QLabel(this);
	m_statusFirewall = new QLabel(this);
	m_statusFirewall->setPixmap(SmallIcon("messagebox_warning"));
	QToolTip::add(m_statusFirewall,i18n("No incoming connections (possibly firewalled)"));

	statusBar()->addWidget(m_statusFirewall);
	statusBar()->addWidget(m_statusInfo,1);
	statusBar()->addWidget(m_statusDHT);
	statusBar()->addWidget(m_statusSpeed);
	statusBar()->addWidget(m_statusTransfer);

	m_statusFirewall->hide(); 

	setupActions();
	currentTorrentChanged(0);
	
	m_dcop = new KTorrentDCOP(this);

	setStandardToolBarMenuEnabled(true);

	QToolTip::add(m_statusInfo, i18n("Info"));
	QToolTip::add(m_statusTransfer, i18n("Data transferred during the current session"));
	QToolTip::add(m_statusSpeed, i18n("Current speed of all torrents combined"));
	
	//first apply settings..
	applySettings(false);
	//then load stuff from core
	m_core->loadTorrents();
	m_core->loadPlugins();

	connect(&m_gui_update_timer, SIGNAL(timeout()), this, SLOT(updatedStats()));
	//Apply GUI update interval
	int val = 500;
	switch(Settings::guiUpdateInterval())
	{
		case 1:
			val = 1000;
			break;
		case 2:
			val = 2000;
			break;
		case 3:
			val = 5000;
			break;
		default:
			val = 500;
	}
	m_gui_update_timer.start(val);
	
	statusBar()->show();
	
	addToolWidget(m_group_view,"player_playlist",i18n("Groups"),DOCK_LEFT);
	
	setAutoSaveSettings("WindowStatus",true);
	KGlobal::config()->setGroup("WindowStatus");
	bool hidden_on_exit = KGlobal::config()->readBoolEntry("hidden_on_exit",false);
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

	bool menubar_hidden = KGlobal::config()->readBoolEntry("menubar_hidden",false);
	menuBar()->setHidden(menubar_hidden);
	m_menubarAction->setChecked(!menubar_hidden);

	bool statusbar_hidden = KGlobal::config()->readBoolEntry("statusbar_hidden",false);
	statusBar()->setHidden(statusbar_hidden);
	m_statusbarAction->setChecked(!statusbar_hidden);

	MaximizeLimits();
	connect(&m_status_msg_expire,SIGNAL(timeout()),this,SLOT(statusBarMsgExpired()));
	
	m_view_man->updateActions();
}

KTorrent::~KTorrent()
{
	delete m_dcop;
	delete m_core;
	delete m_pref;
	delete m_statusInfo;
	delete m_statusTransfer;
	delete m_statusSpeed;
	delete m_statusFirewall;
}

void KTorrent::openView(kt::Group* g)
{
	if (!g)
		return;
	
	KTorrentView* v = m_view_man->newView();
	v->setCurrentGroup((Group*)g);
	v->setupViewColumns();
	
	addTabPage(v,g->groupIcon(),v->caption(),m_view_man);
	
	connect(v,SIGNAL(currentChanged(kt::TorrentInterface* )),
			this,SLOT(currentTorrentChanged(kt::TorrentInterface* )));

	connect(v,SIGNAL(wantToRemove(kt::TorrentInterface*,bool )),
			m_core,SLOT(remove(kt::TorrentInterface*,bool )));

	connect(v->listView(),SIGNAL(dropped(QDropEvent*,QListViewItem*)),
			this,SLOT(urlDropped(QDropEvent*,QListViewItem*)));
	
	connect(v,SIGNAL(wantToStart( kt::TorrentInterface* )),
			m_core,SLOT(start( kt::TorrentInterface* )));
	
	connect(v,SIGNAL(wantToStop( kt::TorrentInterface*, bool )),
			m_core,SLOT(stop( kt::TorrentInterface*, bool )));
	
	connect(v,SIGNAL(needsDataCheck( kt::TorrentInterface* )),
			m_core,SLOT(doDataCheck( kt::TorrentInterface* )));
	
	connect(v,SIGNAL(updateActions( int )),
			this,SLOT(onUpdateActions( int )));
	
	//connect Core queue() with queue() from KTView.
	connect(v, SIGNAL(queue( kt::TorrentInterface* )), 
			m_core, SLOT(queue( kt::TorrentInterface* )));
	
	connect(v,SIGNAL(updateGroupsSubMenu(KPopupMenu*)),
			m_group_view,SLOT(updateGroupsSubMenu(KPopupMenu*)));
	
	connect(v,SIGNAL(groupsSubMenuItemActivated(KTorrentView*, const QString&)),
			m_group_view,SLOT(onGroupsSubMenuItemActivated(KTorrentView*, const QString&)));
	
	if (m_core)
	{
		QueueManager* qman = m_core->getQueueManager();
		QueueManager::iterator i = qman->begin();
		while (i != qman->end())
		{
			v->addTorrent(*i);
			i++;
		}
	}
}

void KTorrent::openView(const QString & group_name)
{
	const kt::Group* g = m_group_view->findGroup(group_name);
	if (g)
		openView((kt::Group*)g);
}

void KTorrent::groupChanged(kt::Group* g)
{
	KTorrentView* v = m_view_man->getCurrentView();
	if (v)
	{
		m_activeTabWidget->changeTab(v,g->groupName());
		v->setIcon(g->groupIcon());
		v->setCurrentGroup(g);
	}
}

void KTorrent::groupRenamed(kt::Group* g)
{
	m_view_man->groupRenamed(g,m_activeTabWidget);
}

void KTorrent::groupRemoved(kt::Group* g)
{
	kt::Group* allg = m_group_view->groupManager()->allGroup();
	m_view_man->groupRemoved(g,m_activeTabWidget,this,allg);
}

void KTorrent::addTabPage(QWidget* page,const QIconSet & icon,
						  const QString & caption,kt::CloseTabListener* ctl)
{
	addWidget(page,caption);
	page->setIcon(icon.pixmap(QIconSet::Small,QIconSet::Active));
	m_tab_map[page] = ctl;
	currentTabChanged(page);
}

void KTorrent::addTabPage(QWidget* page,const QPixmap & icon,
						  const QString & caption,kt::CloseTabListener* ctl)
{
	addWidget(page,caption);
	page->setIcon(icon);
	m_tab_map[page] = ctl;
	currentTabChanged(page);
}


void KTorrent::removeTabPage(QWidget* page)
{
	if (!m_tab_map.contains(page))
		return;
	
	m_tab_map.erase(page);
	page->reparent(0,QPoint());
	removeWidget(page);
}

void KTorrent::addPrefPage(PrefPageInterface* page)
{
	m_pref->addPrefPage(page);
}

void KTorrent::removePrefPage(PrefPageInterface* page)
{
	m_pref->removePrefPage(page);
}

void KTorrent::applySettings(bool change_port)
{
	m_core->setMaxDownloads(Settings::maxDownloads());
	m_core->setMaxSeeds(Settings::maxSeeds());
	PeerManager::setMaxConnections(Settings::maxConnections());
	PeerManager::setMaxTotalConnections(Settings::maxTotalConnections());
	net::SocketMonitor::setDownloadCap(Settings::maxDownloadRate()*1024);
	net::SocketMonitor::setUploadCap(Settings::maxUploadRate()*1024);
	net::SocketMonitor::setSleepTime(Settings::cpuUsage());
	m_core->setKeepSeeding(Settings::keepSeeding());
	mse::StreamSocket::setTOS(Settings::dSCP() << 2);
	mse::StreamSocket::setMaxConnecting(Settings::maxConnectingSockets());
	if (Settings::allwaysDoUploadDataCheck())
		ChunkManager::setMaxChunkSizeForDataCheck(0);
	else
		ChunkManager::setMaxChunkSizeForDataCheck(Settings::maxSizeForUploadDataCheck() * 1024);

	if (Settings::showSystemTrayIcon())
	{
		m_systray_icon->show();
		m_set_max_upload_rate->update();
		m_set_max_download_rate->update();
	}
	else
	{
		m_systray_icon->hide();
	}

	m_core->changeDataDir(Settings::tempDir());
	UDPTrackerSocket::setPort(Settings::udpTrackerPort());
	if (change_port)
		m_core->changePort(Settings::port());
	
	if (Settings::useExternalIP())
		Tracker::setCustomIP(Settings::externalIP());
	else
		Tracker::setCustomIP(QString::null);
        
	Downloader::setMemoryUsage(Settings::memoryUsage());
	Choker::setNumUploadSlots(Settings::numUploadSlots());
	
	//Apply GUI update interval
	int val = 500;
	switch(Settings::guiUpdateInterval())
	{
		case 1:
			val = 1000;
			break;
		case 2:
			val = 2000;
			break;
		case 3:
			val = 5000;
			break;
		default:
			val = 500;
	}
	m_gui_update_timer.changeInterval(val);
	
	//update QM
	m_core->getQueueManager()->orderQueue();
	dht::DHTBase & ht = Globals::instance().getDHT();
	if (Settings::dhtSupport() && !ht.isRunning())
	{
		ht.start(kt::DataDir() + "dht_table",kt::DataDir() + "dht_key",Settings::dhtPort());
	}
	else if (!Settings::dhtSupport() && ht.isRunning())
	{
		ht.stop();
	}
	else if (Settings::dhtSupport() && ht.getPort() != Settings::dhtPort())
	{
		Out(SYS_GEN|LOG_NOTICE) << "Restarting DHT with new port " << Settings::dhtPort() << endl;
		ht.stop();
		ht.start(kt::DataDir() + "dht_table",kt::DataDir() + "dht_key",Settings::dhtPort());
	}
	
	if (Settings::useEncryption())
	{
		Globals::instance().getServer().enableEncryption(Settings::allowUnencryptedConnections());
	}
	else
	{
		Globals::instance().getServer().disableEncryption();
	}
}

void KTorrent::load(const KURL& url)
{
	m_core->load(url);
}

void KTorrent::loadSilently(const KURL& url)
{
	m_core->loadSilently(url);
}

void KTorrent::onUpdateActions(int flags)
{
	m_start->setEnabled(flags & KTorrentView::START);
	m_stop->setEnabled(flags & KTorrentView::STOP);
	m_remove->setEnabled(flags & KTorrentView::REMOVE);
	m_queueaction->setEnabled(flags & KTorrentView::REMOVE);
	m_datacheck->setEnabled(flags & KTorrentView::SCAN);
	m_startall->setEnabled(flags & KTorrentView::START_ALL);
	m_stopall->setEnabled(flags & KTorrentView::STOP_ALL);
}

void KTorrent::currentTorrentChanged(kt::TorrentInterface* tc)
{
	notifyViewListeners(tc);
}


void KTorrent::setupActions()
{
	KStdAction::openNew(this,SLOT(fileNew()),actionCollection());
	KStdAction::open(this, SLOT(fileOpen()), actionCollection());
	KStdAction::quit(kapp, SLOT(quit()), actionCollection());
	
	KStdAction::paste(kapp,SLOT(paste()),actionCollection());

	m_statusbarAction = KStdAction::showStatusbar(this, SLOT(optionsShowStatusbar()), actionCollection());
	m_menubarAction = KStdAction::showMenubar(this, SLOT(optionsShowMenubar()), actionCollection()); 

	KStdAction::keyBindings(this, SLOT(optionsConfigureKeys()), actionCollection());
	KStdAction::configureToolbars(this, SLOT(optionsConfigureToolbars()), actionCollection());

	KAction* pref = KStdAction::preferences(this, SLOT(optionsPreferences()), actionCollection());

	m_start = new KAction(
			i18n("to start", "Start"), "ktstart",0,this, SLOT(startDownload()),
			actionCollection(), "Start");

	m_stop = new KAction(
			i18n("to stop", "Stop"), "ktstop",0,this, SLOT(stopDownload()),
			actionCollection(), "Stop");

	m_remove = new KAction(
			i18n("Remove"), "ktremove",0,this, SLOT(removeDownload()),
			actionCollection(), "Remove");
	
	m_startall = new KAction(
			i18n("to start all", "Start All"), "ktstart_all",0,this, SLOT(startAllDownloadsCurrentView()),
			actionCollection(), "Start all");
	
	m_startall_systray = new KAction(i18n("to start all", "Start All"), "ktstart_all",0,this, SLOT(startAllDownloads()),actionCollection());
	
	m_stopall = new KAction(
			i18n("to stop all", "Stop All"), "ktstop_all",0,this, SLOT(stopAllDownloadsCurrentView()),
			actionCollection(), "Stop all");
	
	m_stopall_systray = new KAction(i18n("to stop all", "Stop All"), "ktstop_all",0,this, SLOT(stopAllDownloads()),actionCollection());
	
	m_pasteurl = new KAction(
			i18n("to paste torrent URL", "Paste Torrent URL..."), "ktstart",0,this, SLOT(torrentPaste()),
			actionCollection(), "paste_url");
	
	m_queuemgr = new KAction(
			i18n("to open Queue Manager", "Open Queue Manager..."),
			"ktqueuemanager", 0, this, SLOT(queueManagerShow()),
			actionCollection(), "Queue manager");
	
	m_queueaction = new KAction(
			i18n("Enqueue/Dequeue"),
			"player_playlist", 0, m_view_man, SLOT(queueAction()),
			actionCollection(), "queue_action");
	
	m_ipfilter = new KAction(
			i18n("IPFilter"),
			"filter", 0, this, SLOT(showIPFilter()),
			actionCollection(), "ipfilter_action");
	
	m_datacheck = new KAction(
			i18n("Check Data Integrity"),
	QString::null,0,m_view_man,SLOT(checkDataIntegrity()),actionCollection(),"check_data");
	
	m_find = KStdAction::find(this,SLOT(find()),actionCollection());
	
	//Plug actions to systemtray context menu
	m_startall_systray->plug(m_systray_icon->contextMenu());
	m_stopall_systray->plug(m_systray_icon->contextMenu());
	m_systray_icon->contextMenu()->insertSeparator();
	m_pasteurl->plug(m_systray_icon->contextMenu());
	m_systray_icon->contextMenu()->insertSeparator();

	m_set_max_upload_rate = new SetMaxRate(m_core, 0, this);
	m_systray_icon->contextMenu()->insertItem(i18n("Set max upload rate"),m_set_max_upload_rate);

	m_set_max_download_rate = new SetMaxRate(m_core, 1, this);
	m_systray_icon->contextMenu()->insertItem(i18n("Set max download rate"),m_set_max_download_rate);

	pref->plug(m_systray_icon->contextMenu());
	
	createGUI(0);
}



bool KTorrent::queryClose()
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

bool KTorrent::queryExit()
{
	// stop timers to prevent update
	m_gui_update_timer.stop();
	
	KGlobal::config()->setGroup("WindowStatus");
	KGlobal::config()->writeEntry("hidden_on_exit",this->isHidden());
	KGlobal::config()->writeEntry("menubar_hidden",menuBar()->isHidden());
	KGlobal::config()->writeEntry("statusbar_hidden",statusBar()->isHidden());
	m_view_man->saveViewState(KGlobal::config());
	saveSettings();
	hide();
	m_systray_icon->hide(); // hide system tray icon upon exit
	m_core->onExit();
	if (Globals::instance().getDHT().isRunning())
		Globals::instance().getDHT().stop();
	return true;
}


void KTorrent::fileNew()
{
	TorrentCreatorDlg dlg(m_core,this);

	dlg.show();
	dlg.exec();
}

void KTorrent::fileOpen()
{
	QString filter = "*.torrent|" + i18n("Torrent Files") + "\n*|" + i18n("All Files");
	KURL url = KFileDialog::getOpenURL(QString::null, filter, this, i18n("Open Location"));

	if (url.isValid())
		load(url);
}

void KTorrent::torrentPaste()
{
	PasteDialog dlg(m_core,this);
	dlg.exec();
}

void KTorrent::queueManagerShow()
{
	QueueDialog dlg(m_core->getQueueManager(), this);
	dlg.exec();
}

void KTorrent::startDownload()
{
	m_view_man->startDownloads();
	TorrentInterface* tc = m_view_man->getCurrentTC();
	currentTorrentChanged(tc);
}

void KTorrent::startAllDownloadsCurrentView()
{
	m_view_man->startAllDownloads();
}

void KTorrent::startAllDownloads()
{
	m_core->startAll(3);
}

void KTorrent::stopDownload()
{
	m_view_man->stopDownloads();
	TorrentInterface* tc = m_view_man->getCurrentTC();	
	currentTorrentChanged(tc);
}

void KTorrent::stopAllDownloads()
{
	m_core->stopAll(3);
}

void KTorrent::stopAllDownloadsCurrentView()
{
	m_view_man->stopAllDownloads();
}

void KTorrent::removeDownload()
{
	m_view_man->removeDownloads();
	currentTorrentChanged(m_view_man->getCurrentTC());
}

void KTorrent::optionsShowStatusbar()
{
	// this is all very cut and paste code for showing/hiding the
	// statusbar
	if (m_statusbarAction->isChecked())
		statusBar()->show();
	else
		statusBar()->hide();
}

void KTorrent::optionsShowMenubar()
{
	// this is all very cut and paste code for showing/hiding the
	// menubar
	if (m_menubarAction->isChecked())
		menuBar()->show();
	else
		menuBar()->hide();
}

void KTorrent::optionsConfigureKeys()
{
	KKeyDialog::configure(actionCollection());
}

void KTorrent::optionsConfigureToolbars()
{
	// use the standard toolbar editor
#if defined(KDE_MAKE_VERSION)
# if KDE_VERSION >= KDE_MAKE_VERSION(3,1,0)
	saveMainWindowSettings(KGlobal::config(), autoSaveGroup());
# else
	saveMainWindowSettings(KGlobal::config());
# endif
#else
	saveMainWindowSettings(KGlobal::config());
#endif
        KEditToolbar dlg(factory());
        connect(&dlg,SIGNAL(newToolbarConfig()),this,SLOT(newToolbarConfig()));
        dlg.exec();
}

void KTorrent::newToolbarConfig()
{
	// this slot is called when user clicks "Ok" or "Apply" in the toolbar editor.
	// recreate our GUI, and re-apply the settings (e.g. "text under icons", etc.)
	createGUI(0);

#if defined(KDE_MAKE_VERSION)
# if KDE_VERSION >= KDE_MAKE_VERSION(3,1,0)
	applyMainWindowSettings(KGlobal::config(), autoSaveGroup());
# else
	applyMainWindowSettings(KGlobal::config());
# endif
#else
	applyMainWindowSettings(KGlobal::config());
#endif
}

void KTorrent::optionsPreferences()
{
	// popup some sort of preference dialog, here
	m_pref->updateData();
	m_pref->exec();
}

void KTorrent::changeStatusbar(const QString& text)
{
	// display the text on the statusbar
	//statusBar()->message(text);
	m_statusInfo->setText(text);
	m_status_msg_expire.stop();
	m_status_msg_expire.start(5000,true);
}

void KTorrent::changeCaption(const QString& text)
{
	// display the text on the caption
	setCaption(text);
}

void KTorrent::saveProperties(KConfig* )
{
}

void KTorrent::readProperties(KConfig*)
{
}

void KTorrent::urlDropped(QDropEvent* event,QListViewItem*)
{
	KURL::List urls;

	if (KURLDrag::decode(event, urls) && !urls.isEmpty())
	{
		for (KURL::List::iterator i = urls.begin();i != urls.end();i++)
			load(*i);
	}
}

void KTorrent::updatedStats()
{
	m_startall_systray->setEnabled(m_core->getNumTorrentsNotRunning() > 0);
	m_stopall_systray->setEnabled(m_core->getNumTorrentsRunning() > 0);
	
	CurrentStats stats = this->m_core->getStats();
	
	//m_statusInfo->setText(i18n("Some info here e.g. connected/disconnected"));
	QString tmp = i18n("Speed down: %1 / up: %2")
			.arg(KBytesPerSecToString((double)stats.download_speed/1024.0))
			.arg(KBytesPerSecToString((double)stats.upload_speed/1024.0));

	m_statusSpeed->setText(tmp);

	QString tmp1 = i18n("Transferred down: %1 / up: %2")
			.arg(BytesToString(stats.bytes_downloaded))
			.arg(BytesToString(stats.bytes_uploaded));
	m_statusTransfer->setText(tmp1);
	
	if (ServerAuthenticate::isFirewalled() && m_core->getNumTorrentsRunning() > 0) 
		m_statusFirewall->show();
	else
		m_statusFirewall->hide();

	m_view_man->update();
	
	m_systray_icon->updateStats(stats,Settings::showSpeedBarInTrayIcon(),Settings::downloadBandwidth(), Settings::uploadBandwidth());
	
	m_core->getPluginManager().updateGuiPlugins();
	

	if (Globals::instance().getDHT().isRunning())
	{
		const dht::Stats & s = Globals::instance().getDHT().getStats();
		m_statusDHT->setText(i18n("DHT: %1 nodes, %2 tasks")
				.arg(s.num_peers).arg(s.num_tasks));
	}
	else
		m_statusDHT->setText(i18n("DHT: off"));
}

void KTorrent::mergePluginGui(Plugin* p)
{
	if (!p) return;
	guiFactory()->addClient(p);
}

void KTorrent::removePluginGui(Plugin* p)
{
	if (!p) return;
	guiFactory()->removeClient(p);
}

void KTorrent::addWidgetInView(QWidget* w,Position pos)
{
}

void KTorrent::removeWidgetFromView(QWidget* w)
{
}

void KTorrent::addWidgetBelowView(QWidget* w,const QString & icon,const QString & caption)
{
	addToolWidget(w,icon,caption,DOCK_BOTTOM);
}
	
void KTorrent::removeWidgetBelowView(QWidget* w)
{
	removeToolWidget(w);
}

void KTorrent::addToolWidget(QWidget* w,const QString & icon,const QString & caption,ToolDock dock)
{
	if (!w) return;
	
	
	if (!icon.isNull())
		w->setIcon(KGlobal::iconLoader()->loadIcon(icon,KIcon::Small));
	
	switch (dock)
	{
		case DOCK_BOTTOM:
			addDockWidget(DDockWindow::Bottom,w,caption);
			break;
			
		case DOCK_LEFT:
			addDockWidget(DDockWindow::Left,w,caption);
			break;
			
		case DOCK_RIGHT:
		default:
			addDockWidget(DDockWindow::Right,w,caption);
			break;
	}
}

void KTorrent::removeToolWidget(QWidget* w)
{
	if (!w) return;
	
	removeDockWidget(w);
	w->reparent(0,QPoint());
}

const TorrentInterface* KTorrent::getCurrentTorrent() const
{
	return m_view_man->getCurrentTC();
}

QString KTorrent::getStatusInfo() 
{
	return m_statusInfo->text();
}

QString KTorrent::getStatusTransfer() 
{
	return m_statusTransfer->text();
}

QString KTorrent::getStatusSpeed() 
{
	return m_statusSpeed->text();
}

QString KTorrent::getStatusDHT() 
{
	return m_statusDHT->text();
}

QString KTorrent::getStatusFirewall()
{
	return m_statusFirewall->text();
}

QCStringList KTorrent::getTorrentInfo(kt::TorrentInterface* tc)
{
	return KTorrentViewItem::getTorrentInfo(tc);
}

void KTorrent::showIPFilter()
{
	IPFilterWidget ipf(this);
	ipf.exec();
}


void KTorrent::closeTab()
{
	QWidget* w = m_currentWidget;
	if (!w)
		return;
	
	CloseTabListener* ctl = m_tab_map[w];
	if (ctl)
	{
		ctl->tabCloseRequest(this,w);
		currentTabChanged(m_activeTabWidget->currentPage());
	}
}

void KTorrent::currentTabChanged(QWidget* w)
{
	m_view_man->onCurrentTabChanged(w);
	currentTorrentChanged(m_view_man->getCurrentTC());
	if (!m_activeTabWidget || !w)
		return;
	
	bool close_allowed = false;
	CloseTabListener* ctl = m_tab_map[w];
	if (ctl)
		close_allowed = ctl->closeAllowed(w);
	
	m_activeTabWidget->closeButton()->setEnabled(close_allowed);
}

void KTorrent::openDefaultView()
{
	openView(i18n("All Torrents"));
}

KToolBar* KTorrent::addToolBar(const char* name)
{
	return toolBar(name);
}

void KTorrent::removeToolBar(KToolBar* tb)
{
	delete tb;
}

void KTorrent::loadSilentlyDir(const KURL& url, const KURL& savedir)
{
	m_core->loadSilentlyDir(url, savedir);
}

KProgress* KTorrent::addProgressBarToStatusBar()
{
	if (m_status_prog)
		return 0;
	
	KStatusBar* sb = statusBar();
	m_status_prog = new KProgress(100,sb);
	m_status_prog->setValue(0);
	sb->addWidget(m_status_prog);
	m_status_prog->show();
	return m_status_prog;
}

void KTorrent::removeProgressBarFromStatusBar(KProgress* p)
{
	if (m_status_prog != p)
		return;
	
	KStatusBar* sb = statusBar();
	sb->removeWidget(p);
	delete p;
	m_status_prog = 0;
}

void KTorrent::statusBarMsgExpired()
{
	m_statusInfo->clear();
}

void KTorrent::find()
{
	KTorrentView* v = m_view_man->getCurrentView();
	if (v)
		v->toggleFilterBar();
}

#include "ktorrent.moc"

