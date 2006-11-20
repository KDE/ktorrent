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
#include <expandablewidget.h>
#include <pluginmanager.h>
#include <groups/group.h>
#include <groups/groupview.h>
#include <mse/streamsocket.h>



namespace kt
{
	QString DataDir();
}

#include <util/profiler.h>



using namespace bt;
using namespace kt;

KTorrent::KTorrent()
	: KMdiMainFrm( 0, "KTorrent",KMdi::IDEAlMode ),m_group_view(0),
		m_view(0), m_systray_icon(0)
{
	setHidden(true);
	setToolviewStyle(KMdi::TextAndIcon);
	
	KIconLoader* iload = KGlobal::iconLoader();
	
	m_view = new KTorrentView(0);
	m_view_exp = new ExpandableWidget(m_view,0);
	addTabPage(m_view_exp,iload->loadIconSet("folder", KIcon::Small),i18n("Torrents"));
	
	m_group_view = new kt::GroupView(m_view,actionCollection());
	m_group_view->loadGroups();

	m_pref = new KTorrentPreferences(*this);
	m_core = new KTorrentCore(this);
	m_systray_icon = new TrayIcon(m_core, this);
	
	connect(m_group_view,SIGNAL(currentGroupChanged( kt::Group* )),
			this,SLOT(groupChanged(kt::Group*)));
	
	connect(m_group_view,SIGNAL(groupRenamed(kt::Group*)),
			this,SLOT(groupChanged(kt::Group*)));
	
	connect(m_core, SIGNAL(finished( kt::TorrentInterface* )), 
			m_view, SLOT(torrentFinished( kt::TorrentInterface* )));

	connect(m_core,SIGNAL(torrentAdded(kt::TorrentInterface* )),
			m_view,SLOT(addTorrent(kt::TorrentInterface* )));

	connect(m_core,SIGNAL(torrentRemoved(kt::TorrentInterface* )),
			m_view,SLOT(removeTorrent(kt::TorrentInterface* )));

	connect(m_view,SIGNAL(currentChanged(kt::TorrentInterface* )),
			this,SLOT(currentTorrentChanged(kt::TorrentInterface* )));

	connect(m_view,SIGNAL(wantToRemove(kt::TorrentInterface*,bool )),
			m_core,SLOT(remove(kt::TorrentInterface*,bool )));

	connect(m_view,SIGNAL(dropped(QDropEvent*,QListViewItem*)),
			this,SLOT(urlDropped(QDropEvent*,QListViewItem*)));
	
	connect(m_view,SIGNAL(wantToStart( kt::TorrentInterface* )),
			m_core,SLOT(start( kt::TorrentInterface* )));
	
	connect(m_view,SIGNAL(wantToStop( kt::TorrentInterface*, bool )),
			m_core,SLOT(stop( kt::TorrentInterface*, bool )));
	
	connect(m_view,SIGNAL(needsDataCheck( kt::TorrentInterface* )),
			m_core,SLOT(doDataCheck( kt::TorrentInterface* )));
	
	connect(m_view,SIGNAL(updateActions( bool, bool, bool, bool )),
			this,SLOT(onUpdateActions( bool, bool, bool, bool )));
	
	//connect Core queue() with queue() from KTView.
	connect(m_view, SIGNAL(queue( kt::TorrentInterface* )), 
			m_core, SLOT(queue( kt::TorrentInterface* )));
	
	connect(m_core, SIGNAL(torrentRemoved( kt::TorrentInterface* )),
			m_group_view, SLOT(onTorrentRemoved( kt::TorrentInterface* )));
	

	m_statusInfo = new KSqueezedTextLabel(this);
	m_statusSpeed = new QLabel(this);
	m_statusTransfer = new QLabel(this);
	m_statusDHT = new QLabel(this);
	


	statusBar()->addWidget(m_statusInfo,1);
	statusBar()->addWidget(m_statusDHT);
	statusBar()->addWidget(m_statusSpeed);
	statusBar()->addWidget(m_statusTransfer);

	
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
	
	groupChanged(m_group_view->currentGroup());
	tabWidget()->setHoverCloseButton(false);
	
	m_close_cur_tab = new KPushButton(KGuiItem(QString::null,"tab_remove"),tabWidget());
	connect(m_close_cur_tab,SIGNAL(clicked()),this,SLOT(tabClosePressed()));
	tabWidget()->setCornerWidget(m_close_cur_tab,Qt::TopRight);
	connect(tabWidget(),SIGNAL(currentChanged(QWidget*)),this,SLOT(currentTabChanged( QWidget* )));
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
}

KTorrent::~KTorrent()
{
	while (m_tab_map.count() > 0)
	{
		QMap<QWidget*,Tab>::iterator itr = m_tab_map.begin();
		removeTabPage(itr.key());
	}
	delete m_dcop;
	delete m_core;
	delete m_pref;
	delete m_statusInfo;
	delete m_statusTransfer;
	delete m_statusSpeed;
	delete m_view_exp;
}

void KTorrent::groupChanged(kt::Group* g)
{
	if (!m_tab_map.contains(m_view_exp))
		return;
	
	KMdiChildView* cv = m_tab_map[m_view_exp].child;
	if (cv)
	{
		cv->setCaption(g->groupName());
		cv->setMDICaption(g->groupName());
		cv->setTabCaption(g->groupName());
		cv->setIcon(g->groupIcon());
	}
	m_view->setCurrentGroup(g);
}

void KTorrent::addTabPage(QWidget* page,const QIconSet & icon,
						  const QString & caption,kt::CloseTabListener* ctl)
{
	KMdiChildView* cv = createWrapper(page,caption,caption);
	addWindow(cv);
	cv->setIcon(icon.pixmap(QIconSet::Small,QIconSet::Active));
	Tab t;
	t.child = cv;
	t.ctl = ctl;
	m_tab_map.insert(page,t);
}

void KTorrent::removeTabPage(QWidget* page)
{
	if (!m_tab_map.contains(page))
		return;
	
	KMdiChildView* cv = m_tab_map[page].child;
	m_tab_map.erase(page);
	page->reparent(0,QPoint());
	closeWindow(cv);
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
//	UploadCap::instance().setMaxSpeed(Settings::maxUploadRate() * 1024);
//	DownloadCap::instance().setMaxSpeed(Settings::maxDownloadRate()*1024);
	net::SocketMonitor::setDownloadCap(Settings::maxDownloadRate()*1024);
	net::SocketMonitor::setUploadCap(Settings::maxUploadRate()*1024);
	m_core->setKeepSeeding(Settings::keepSeeding());
	mse::StreamSocket::setTOS(Settings::typeOfService());
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
		ht.start(kt::DataDir() + "dht_table",Settings::dhtPort());
	}
	else if (!Settings::dhtSupport() && ht.isRunning())
	{
		ht.stop();
	}
	else if (Settings::dhtSupport() && ht.getPort() != Settings::dhtPort())
	{
		Out(SYS_GEN|LOG_NOTICE) << "Restarting DHT with new port " << Settings::dhtPort() << endl;
		ht.stop();
		ht.start(kt::DataDir() + "dht_table",Settings::dhtPort());
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

void KTorrent::onUpdateActions(bool can_start,bool can_stop,bool can_remove,bool can_scan)
{
	m_start->setEnabled(can_start);
	m_stop->setEnabled(can_stop);
	m_remove->setEnabled(can_remove);
	m_queueaction->setEnabled(can_remove);
	m_datacheck->setEnabled(can_scan);
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
			i18n("to start all", "Start All"), "ktstart_all",0,this, SLOT(startAllDownloads()),
			actionCollection(), "Start all");
	
	m_stopall = new KAction(
			i18n("to stop all", "Stop All"), "ktstop_all",0,this, SLOT(stopAllDownloads()),
			actionCollection(), "Stop all");
	
	m_pasteurl = new KAction(
			i18n("to paste torrent URL", "Paste Torrent URL..."), "ktstart",0,this, SLOT(torrentPaste()),
			actionCollection(), "paste_url");
	
	m_queuemgr = new KAction(
			i18n("to open Queue Manager", "Open Queue Manager..."),
			"ktqueuemanager", 0, this, SLOT(queueManagerShow()),
			actionCollection(), "Queue manager");
	
	m_queueaction = new KAction(
			i18n("Enqueue/Dequeue"),
			"player_playlist", 0, this, SLOT(queueAction()),
			actionCollection(), "queue_action");
	
	m_ipfilter = new KAction(
			i18n("IPFilter"),
			"filter", 0, this, SLOT(showIPFilter()),
			actionCollection(), "ipfilter_action");
	
	m_datacheck = new KAction(
			i18n("Check Data Integrity"),
	QString::null,0,this,SLOT(checkDataIntegrity()),actionCollection(),"check_data");
	
	//Plug actions to systemtray context menu
	m_startall->plug(m_systray_icon->contextMenu());
	m_stopall->plug(m_systray_icon->contextMenu());
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
#ifdef KT_PROFILE
	bt::Profiler::instance().saveToFile(kt::DataDir() + "profile");
#endif
	m_group_view->saveGroups();
	// stop timers to prevent update
	m_gui_update_timer.stop();
	m_core->onExit();
	if (Globals::instance().getDHT().isRunning())
		Globals::instance().getDHT().stop();
	
	KGlobal::config()->setGroup("WindowStatus");
	KGlobal::config()->writeEntry( "hidden_on_exit",this->isHidden());
	m_view->saveSettings();
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
	m_view->startDownloads();
	TorrentInterface* tc = m_view->getCurrentTC();
	
	currentTorrentChanged(tc);
	m_view->onSelectionChanged(); // trigger an updateActions signal
}

void KTorrent::startAllDownloads()
{
	m_view->startAllDownloads();
	m_view->onSelectionChanged(); // trigger an updateActions signal
}

void KTorrent::stopDownload()
{
	m_view->stopDownloads();
			
	TorrentInterface* tc = m_view->getCurrentTC();	
	currentTorrentChanged(tc);
	m_view->onSelectionChanged(); // trigger an updateActions signal
}

void KTorrent::stopAllDownloads()
{
	m_view->stopAllDownloads();
	m_view->onSelectionChanged(); // trigger an updateActions signal
}

void KTorrent::removeDownload()
{
	m_view->removeDownloads();
			
	TorrentInterface* tc = m_view->getCurrentTC();
	currentTorrentChanged(tc);
	notifyViewListeners(tc);
	m_view->onSelectionChanged(); // trigger an updateActions signal
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
}

void KTorrent::changeCaption(const QString& text)
{
	// display the text on the caption
	setCaption(text);
}

void KTorrent::saveProperties(KConfig* )
{
	KGlobal::config()->writeEntry( "hidden_on_exit",this->isHidden());
	KGlobal::config()->sync();
}

void KTorrent::readProperties(KConfig*)
{
	bool hidden_on_exit = KGlobal::config()->readBoolEntry("hidden_on_exit",false);
	if (!(Settings::showSystemTrayIcon() && hidden_on_exit))
	{
		Out(SYS_GEN|LOG_DEBUG) << "Showing KT" << endl;
		show();
	}
	else
	{
		hide();
	}
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
	KT_PROF_START("updatedStats");
	KT_PROF_START("misc");
	m_startall->setEnabled(m_core->getNumTorrentsNotRunning() > 0);
	m_stopall->setEnabled(m_core->getNumTorrentsRunning() > 0);
	
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
	KT_PROF_END();
	
	KT_PROF_START("view");
	m_view->update();
	KT_PROF_END();
	
	KT_PROF_START("systray");
	m_systray_icon->updateStats(stats,Settings::showSpeedBarInTrayIcon(),Settings::downloadBandwidth(), Settings::uploadBandwidth());
	KT_PROF_END();
	
	KT_PROF_START("plugins");
	m_core->getPluginManager().updateGuiPlugins();
	KT_PROF_END();
	
#if 0

	//update tab labels
	QString tabText = i18n("Downloads %1/%2").arg(m_core->getNumRunning(true)).arg(m_core->countDownloads());
	//kdDebug() << "tabtext: " << tabText << " " << m_tabs->tabLabel(m_view_exp) << endl;
	if (tabText != tabWidget()->tabLabel(m_view_exp).replace('&', ""))
		tabWidget()->setTabLabel(m_view_exp, tabText);
		
	tabText = i18n("Uploads %1/%2").arg(m_core->getNumRunning(false,true)).arg(m_core->countSeeds());
	if (tabText != m_tabs->tabLabel(m_seedView_exp).replace('&', ""))
		m_tabs->setTabLabel(m_seedView_exp, tabText);
#endif

	KT_PROF_START("DHT");
	if (Globals::instance().getDHT().isRunning())
	{
		const dht::Stats & s = Globals::instance().getDHT().getStats();
		m_statusDHT->setText(i18n("DHT: %1 nodes, %2 tasks")
				.arg(s.num_peers).arg(s.num_tasks));
	}
	else
		m_statusDHT->setText(i18n("DHT: off"));
	KT_PROF_END();
	KT_PROF_END();
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
	if (!w) return;
	
	m_view_exp->expand(w,pos);
}

void KTorrent::removeWidgetFromView(QWidget* w)
{
	if (!w) return;
	
	m_view_exp->remove(w);
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
	
	KMdiToolViewAccessor* tw = createToolWindow();
	if (!icon.isNull())
		w->setIcon(KGlobal::iconLoader()->loadIcon(icon,KIcon::Small));
	tw->setWidgetToWrap(w,caption,caption);
	switch (dock)
	{
		case DOCK_BOTTOM:
			tw->place(KDockWidget::DockBottom,getMainDockWidget(),25);
			break;
			
		case DOCK_LEFT:
			tw->place(KDockWidget::DockLeft,getMainDockWidget(),25);
			break;
			
		case DOCK_RIGHT:
		default:
			tw->place(KDockWidget::DockRight,getMainDockWidget(),25);
			break;
	}
}

void KTorrent::removeToolWidget(QWidget* w)
{
	if (!w) return;
	
	w->reparent(0,QPoint());
	deleteToolWindow(w);
}

const TorrentInterface* KTorrent::getCurrentTorrent() const
{
	return m_view->getCurrentTC();
}

void KTorrent::queueAction()
{
	m_view->queueSlot();
}

void KTorrent::checkDataIntegrity()
{
	m_view->checkDataIntegrity();
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

QCStringList KTorrent::getTorrentInfo(kt::TorrentInterface* tc)
{
	return m_view->getTorrentInfo(tc);
}

void KTorrent::showIPFilter()
{
	IPFilterWidget ipf(this);
	ipf.exec();
}

void KTorrent::tabClosePressed()
{
	KMdiChildView* child = activeWindow();
	if (!child)
		return;
	
	QMap<QWidget*,Tab>::iterator i = m_tab_map.begin();
	while (i != m_tab_map.end())
	{
		if (i.data().child == child)
		{
			kt::CloseTabListener* ctl = i.data().ctl;
			ctl->tabCloseRequest(i.key());
			return; 
		}
		i++;
	}
}

void KTorrent::currentTabChanged(QWidget* )
{
	KMdiChildView* child = activeWindow();
	if (!child)
		return;
	
	QMap<QWidget*,Tab>::iterator i = m_tab_map.begin();
	while (i != m_tab_map.end())
	{
		if (i.data().child == child)
		{
			m_close_cur_tab->setEnabled(i.data().ctl != 0);
			return;
		}
		i++;
	}
	m_close_cur_tab->setEnabled(false);
}

#include "ktorrent.moc"

