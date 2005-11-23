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
 *   51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.             *
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

#include <kstdaccel.h>
#include <kaction.h>
#include <kstdaction.h>

#include <interfaces/torrentinterface.h>
#include <torrent/peermanager.h>
#include <torrent/uploadcap.h>
#include <torrent/downloadcap.h>
#include <util/error.h>
#include <torrent/globals.h>
#include <torrent/tracker.h>
#include <torrent/udptrackersocket.h>
#include <util/log.h>
#include <util/fileops.h>

#include "ktorrentcore.h"
#include "ktorrentview.h"
#include "ktorrent.h"
#include "pref.h"
#include "settings.h"
#include "trayicon.h"
#include "ktorrentdcop.h"
#include "torrentcreatordlg.h"
#include "pastedialog.h"
#include <util/functions.h>
#include <interfaces/functions.h>
#include <interfaces/plugin.h>
#include <interfaces/prefpageinterface.h>
#include <expandablewidget.h>
#include <pluginmanager.h>


using namespace bt;
using namespace kt;

KTorrent::KTorrent()
		: KMainWindow( 0, "KTorrent" ),
		m_view(0),m_systray_icon(0)
{
	bool debug = bt::Globals::instance().isDebugModeSet();
	KIconLoader* iload = KGlobal::iconLoader();

	m_tabs = new KTabWidget(0);
	m_view = new KTorrentView(0);
	m_exp = new ExpandableWidget(m_tabs,this);
	m_view_exp = new ExpandableWidget(m_view,m_tabs);
	m_tabs->addTab(m_view_exp,iload->loadIconSet("down", KIcon::Small),i18n("Downloads"));

	m_pref = new KTorrentPreferences(*this);
	m_core = new KTorrentCore(this);
	m_systray_icon = new TrayIcon(m_core, this); 

	connect(m_core,SIGNAL(torrentAdded(kt::TorrentInterface* )),
			m_view,SLOT(addTorrent(kt::TorrentInterface* )));

	connect(m_core,SIGNAL(torrentRemoved(kt::TorrentInterface* )),
			m_view,SLOT(removeTorrent(kt::TorrentInterface* )));

	connect(m_view,SIGNAL(currentChanged(kt::TorrentInterface* )),
			this,SLOT(currentChanged(kt::TorrentInterface* )));

	connect(m_view,SIGNAL(wantToRemove(kt::TorrentInterface*,bool )),
			m_core,SLOT(remove(kt::TorrentInterface*,bool )));


	connect(m_view,SIGNAL(dropped(QDropEvent*,QListViewItem*)),
			this,SLOT(urlDropped(QDropEvent*,QListViewItem*)));

	// then, setup our actions
	setupActions();

	// and a status bar
	statusBar()->show();

	// apply the saved mainwindow settings, if any, and ask the mainwindow
	// to automatically save settings if changed: window size, toolbar
	// position, icon size, etc.
	setAutoSaveSettings();

	currentChanged(0);
	applySettings(false);

	setCentralWidget(m_exp);
	if (debug)
	{
		//m_exp->expand(new LogViewer(bt::Globals::instance().getLog(),0),kt::BELOW);
	}

	m_dcop = new KTorrentDCOP(this);

	m_core->loadTorrents();
	setStandardToolBarMenuEnabled(true);

	m_statusInfo = new KSqueezedTextLabel(this);
	m_statusSpeed = new QLabel(this);
	m_statusTransfer = new QLabel(this);
	


	statusBar()->addWidget(m_statusInfo,1);
	statusBar()->addWidget(m_statusSpeed);
	statusBar()->addWidget(m_statusTransfer);

	QToolTip::add(m_statusInfo, i18n("Info"));
	QToolTip::add(m_statusTransfer, i18n("Data transfered during the current session"));
	QToolTip::add(m_statusSpeed, i18n("Current speed of all torrents combined"));

	connect(&m_gui_update_timer, SIGNAL(timeout()), this, SLOT(updatedStats()));
	m_gui_update_timer.start(500);

	bool hidden_on_exit = KGlobal::config()->readBoolEntry("hidden_on_exit",false);
	if (!(Settings::showSystemTrayIcon() && hidden_on_exit))
	{
		show();
	}
}

KTorrent::~KTorrent()
{
	delete m_dcop;
	bt::Out() << "I'm dead" << bt::endl;
	delete m_core;
	delete m_pref;
	delete m_statusInfo;
	delete m_statusTransfer;
	delete m_statusSpeed;
}

void KTorrent::addTabPage(QWidget* page,const QIconSet & icon,const QString & caption)
{
	m_tabs->addTab(page,icon,caption);
	page->show();
}

void KTorrent::removeTabPage(QWidget* page)
{
	m_tabs->removePage(page);
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
	UploadCap::instance().setMaxSpeed(Settings::maxUploadRate() * 1024);
	DownloadCap::instance().setMaxSpeed(Settings::maxDownloadRate()*1024);
	m_core->setKeepSeeding(Settings::keepSeeding());

	if (Settings::showSystemTrayIcon())
	{
		m_systray_icon->show();
	}
	else
	{
		m_systray_icon->hide();
	}

	m_core->changeDataDir(Settings::tempDir());
	UDPTrackerSocket::setPort(Settings::udpTrackerPort());
	if (change_port)
		m_core->changePort(Settings::port());
	
	Tracker::setCustomIP(Settings::externalIP());
}

void KTorrent::load(const KURL& url)
{
	m_core->load(url);
}

void KTorrent::currentChanged(kt::TorrentInterface* tc)
{
	if (tc)
	{
		const TorrentStats & s = tc->getStats();
		m_start->setEnabled(!s.running);
		m_stop->setEnabled(s.running);
		m_remove->setEnabled(true);
	}
	else
	{
		m_start->setEnabled(false);
		m_stop->setEnabled(false);
		m_remove->setEnabled(false);
	}

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

	pref->plug(m_systray_icon->contextMenu(),1);


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
	
	createGUI();
}



bool KTorrent::queryClose()
{
	if (Settings::showSystemTrayIcon())
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
	m_core->onExit();
	KGlobal::config()->writeEntry( "hidden_on_exit",this->isHidden());
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
	dlg.show();
	dlg.exec();
}

void KTorrent::startDownload()
{
	TorrentInterface* tc = m_view->getCurrentTC();
	if (tc && !tc->getStats().running)
	{
		m_core->start(tc);
		if (!tc->getStats().running)
		{
			bool seed = tc->getStats().bytes_left == 0;
			int nr = seed ? Settings::maxSeeds() : Settings::maxDownloads();
			
			if(!seed)
				KMessageBox::error(this,
								   i18n("Cannot start more than 1 download."
										   " Go to Settings -> Configure KTorrent,"
										   " if you want to change the limit.",
								   			"Cannot start more than %n downloads."
										   " Go to Settings -> Configure KTorrent,"
										   " if you want to change the limit.",
											nr),
								   	i18n("Error"));
			else
				KMessageBox::error(this,
								   i18n("Cannot start more than 1 seed."
										   " Go to Settings -> Configure KTorrent,"
										   " if you want to change the limit.",
								   "Cannot start more than %n seeds."
										   " Go to Settings -> Configure KTorrent,"
										   " if you want to change the limit.",
								   nr),
								   i18n("Error"));
		}
		currentChanged(tc);
	}
}

void KTorrent::startAllDownloads()
{
	m_core->startAll();
}

void KTorrent::stopDownload()
{
	TorrentInterface* tc = m_view->getCurrentTC();
	if (tc && tc->getStats().running)
	{
		tc->stop(true);
		currentChanged(tc);
	}
}

void KTorrent::stopAllDownloads()
{
	m_core->stopAll();
}

void KTorrent::removeDownload()
{
	TorrentInterface* tc = m_view->getCurrentTC();
	if (tc)
	{
		const TorrentStats & s = tc->getStats();
		bool data_to = false;
		if (s.bytes_left > 0)
		{
			QString msg = i18n("The torrent %1 has not finished downloading, "
					"do you want to delete the incomplete data too ?").arg(s.torrent_name);
			int ret = KMessageBox::questionYesNoCancel(this,msg,i18n("Remove Download"));
			if (ret == KMessageBox::Cancel)
				return;
			else if (ret == KMessageBox::Yes)
				data_to = true;
		}
		m_view->removeTorrent(tc);
		m_core->remove(tc,data_to);
		currentChanged(m_view->getCurrentTC());
		notifyViewListeners(m_view->getCurrentTC());
	}
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
	createGUI();

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
	TorrentInterface* tc = m_view->getCurrentTC();
	if (tc)
	{
		const TorrentStats & s = tc->getStats();
		m_start->setEnabled(!s.running);
		m_stop->setEnabled(s.running);
		m_remove->setEnabled(true);
	}
	else
	{
		m_start->setEnabled(false);
		m_stop->setEnabled(false);
		m_remove->setEnabled(false);
	}

	m_startall->setEnabled(m_core->getNumTorrentsNotRunning() > 0);
	m_stopall->setEnabled(m_core->getNumTorrentsRunning() > 0);
	
	CurrentStats stats = this->m_core->getStats();

	//m_statusInfo->setText(i18n("Some info here e.g. connected/disconnected"));
	QString tmp = i18n("Speed up: %1 / down: %2")
			.arg(KBytesPerSecToString((double)stats.upload_speed/1024.0))
			.arg(KBytesPerSecToString((double)stats.download_speed/1024.0));

	m_statusSpeed->setText(tmp);

	QString tmp1 = i18n("Transfered up: %1 / down: %2")
			.arg(BytesToString(stats.bytes_uploaded))
			.arg(BytesToString(stats.bytes_downloaded));
	m_statusTransfer->setText(tmp1);

	m_view->update();
	m_systray_icon->updateStats(tmp + "<br>" + tmp1 + "<br>");
	m_core->getPluginManager().updateGuiPlugins();
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

void KTorrent::addWidgetBelowView(QWidget* w)
{
	if (!w) return;
	
	m_exp->expand(w,kt::BELOW);
}
	
void KTorrent::removeWidgetBelowView(QWidget* w)
{
	if (!w) return;
	
	m_exp->remove(w);
}

const TorrentInterface* KTorrent::getCurrentTorrent() const
{
	return m_view->getCurrentTC();
}

#include "ktorrent.moc"
