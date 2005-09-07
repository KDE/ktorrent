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

#include <libtorrent/torrentcontrol.h>
#include <libtorrent/peermanager.h>
#include <libtorrent/uploadcap.h>
#include <libtorrent/downloadcap.h>
#include <libutil/error.h>
#include <libtorrent/globals.h>
#include <libtorrent/udptrackersocket.h>
#include <libutil/log.h>
#include <libutil/fileops.h>

#include "ktorrentcore.h"
#include "ktorrentview.h"
#include "ktorrent.h"
#include "pref.h"
#include "settings.h"
#include "trayicon.h"
#include "searchwidget.h"
#include "logviewer.h"
#include "ktorrentdcop.h"
#include "torrentcreatordlg.h"
#include "infowidget.h"
#include "functions.h"


using namespace bt;

KTorrent::KTorrent()
		: KMainWindow( 0, "KTorrent" ),
		m_view(0),m_systray_icon(0)
{
	bool debug = bt::Globals::instance().isDebugModeSet();
	QSplitter* s = 0;
	if (debug)
		s = new QSplitter(QSplitter::Vertical,this);

	m_tabs = new KTabWidget(debug ? (QWidget*)s : (QWidget*)this);

	//m_tabs = new KTabWidget(this);
	QSplitter* vbox = new QSplitter(QSplitter::Vertical,m_tabs);
	m_view = new KTorrentView(vbox);
	m_info = new InfoWidget(vbox);
	vbox->moveToFirst(m_view);
	vbox->moveToLast(m_info);

	m_search = new SearchWidget(m_tabs);
	m_core = new KTorrentCore();
	m_systray_icon = new TrayIcon(m_core, this); 


	KIconLoader* iload = KGlobal::iconLoader();
	m_tabs->addTab(vbox,iload->loadIconSet("down", KIcon::Small),i18n("Downloads"));
	m_tabs->addTab(m_search,iload->loadIconSet("viewmag", KIcon::Small),i18n("Search"));

	connect(m_core,SIGNAL(torrentAdded(bt::TorrentControl* )),
			m_view,SLOT(addTorrent(bt::TorrentControl* )));

	connect(m_core,SIGNAL(torrentRemoved(bt::TorrentControl* )),
			m_view,SLOT(removeTorrent(bt::TorrentControl* )));

	connect(m_view,SIGNAL(currentChanged(bt::TorrentControl* )),
			this,SLOT(currentChanged(bt::TorrentControl* )));

	connect(m_core,SIGNAL(finished(bt::TorrentControl* )),
			this,SLOT(askAndSave(bt::TorrentControl* )));

	connect(m_view,SIGNAL(wantToRemove(bt::TorrentControl* )),
			m_core,SLOT(remove(bt::TorrentControl* )));


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
	applySettings();

	if (!debug)
	{
		setCentralWidget(m_tabs);
	}
	else
	{
		s->moveToFirst(m_tabs);
		s->moveToLast(new LogViewer(bt::Globals::instance().getLog(),s));
		setCentralWidget(s);
	}



	connect(m_search,SIGNAL(statusBarMsg(const QString& )),this,SLOT(changeStatusbar(const QString& )));
	connect(m_search,SIGNAL(openTorrent(const KURL& )),this,SLOT(load(const KURL& )));
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
}

KTorrent::~KTorrent()
{
	delete m_dcop;
	bt::Out() << "I'm dead" << bt::endl;
	delete m_core;
//	delete m_mon;
	delete m_statusInfo;
	delete m_statusTransfer;
	delete m_statusSpeed;
}

void KTorrent::applySettings()
{
	m_core->setMaxDownloads(Settings::maxDownloads());
	PeerManager::setMaxConnections(Settings::maxConnections());
	UploadCap::setSpeed(Settings::maxUploadRate() * 1024);
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
	m_core->changePort(Settings::port());
	m_search->loadSearchEngines();
}

void KTorrent::load(const KURL& url)
{
	QString target;
	// download the contents
	if (KIO::NetAccess::download(url,target,this))
	{
		// load in the file (target is always local)
		m_core->load(target);
		// and remove the temp file
		KIO::NetAccess::removeTempFile(target);
	}
	else
	{
		KMessageBox::error(this,KIO::NetAccess::lastErrorString(),i18n("Error"));
	}
}

void KTorrent::currentChanged(bt::TorrentControl* tc)
{
	if (tc)
	{
		bool completed = tc->getBytesLeft() == 0;
		m_save->setEnabled(completed && !tc->isSaved());
		m_start->setEnabled(!tc->isRunning());
		m_stop->setEnabled(tc->isRunning());
		m_remove->setEnabled(true);
	}
	else
	{
		m_save->setEnabled(false);
		m_start->setEnabled(false);
		m_stop->setEnabled(false);
		m_remove->setEnabled(false);
	}
	m_info->changeTC(tc);
}

void KTorrent::setupActions()
{
	KStdAction::openNew(this,SLOT(fileNew()),actionCollection());
	KStdAction::open(this, SLOT(fileOpen()), actionCollection());
	KStdAction::quit(kapp, SLOT(quit()), actionCollection());
	KAction* copy_act = KStdAction::copy(m_search,SLOT(copy()),actionCollection());
	KStdAction::paste(kapp,SLOT(paste()),actionCollection());

	copy_act->plug(m_search->rightClickMenu(),0);
	
	m_save = KStdAction::save(this, SLOT(fileSave()), actionCollection());

	m_statusbarAction = KStdAction::showStatusbar(this, SLOT(optionsShowStatusbar()), actionCollection());

	KStdAction::keyBindings(this, SLOT(optionsConfigureKeys()), actionCollection());
	KStdAction::configureToolbars(this, SLOT(optionsConfigureToolbars()), actionCollection());

	KAction* pref = KStdAction::preferences(this, SLOT(optionsPreferences()), actionCollection());

	pref->plug(m_systray_icon->contextMenu(),1);


	m_start = new KAction(
			i18n("to start", "Start"), "player_play",0,this, SLOT(startDownload()),
			actionCollection(), "Start");

	m_stop = new KAction(
			i18n("to stop", "Stop"), "player_stop",0,this, SLOT(stopDownload()),
			actionCollection(), "Stop");

	m_remove = new KAction(
			i18n("Remove"), "remove",0,this, SLOT(removeDownload()),
			actionCollection(), "Remove");

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

void KTorrent::save(bt::TorrentControl* tc)
{
	if (!tc || tc->getBytesLeft() != 0)
		return;


	try
	{
		QString dir = KFileDialog::getExistingDirectory(QString::null, this,
				i18n("Select Folder to Save To"));


		if (dir != QString::null)
		{
			tc->reconstruct(dir);
		}
	}
	catch (bt::Error & err)
	{
		KMessageBox::error(this,err.toString(),i18n("Error"));
	}
}

void KTorrent::askAndSave(bt::TorrentControl* tc)
{
	if (Settings::saveDir() == QString::null || !bt::Exists(Settings::saveDir()))
	{
		int ret = KMessageBox::questionYesNo(
			this,
			i18n("The download %1 has finished. Do you want to save it now?")
				.arg(tc->getTorrentName()),
			i18n("Save Torrent?"),KStdGuiItem::save(),i18n("Do Not Save"));

		if (ret == KMessageBox::Yes)
			save(tc);
	}
	else
	{
		try
		{
			QString dir = Settings::saveDir();
			tc->reconstruct(dir);
		}
		catch (bt::Error & err)
		{
			KMessageBox::error(0,err.toString(), i18n("Error"));
		}
	}

	currentChanged(m_view->getCurrentTC());
}

void KTorrent::fileSave()
{
	TorrentControl* tc = m_view->getCurrentTC();
	save(tc);
	currentChanged(m_view->getCurrentTC());
}

void KTorrent::startDownload()
{
	TorrentControl* tc = m_view->getCurrentTC();
	if (tc && !tc->isRunning())
	{
		m_core->start(tc);
		if (!tc->isRunning())
		{
			KMessageBox::error(this,
				i18n("Cannot start more than 1 download."
					" Go to Settings -> Configure KTorrent,"
					" if you want to change the limit.",
				     "Cannot start more than %n downloads."
					" Go to Settings -> Configure KTorrent,"
					" if you want to change the limit.",
				    Settings::maxDownloads()),
				i18n("Error"));
		}
		currentChanged(tc);
	}
}

void KTorrent::stopDownload()
{
	TorrentControl* tc = m_view->getCurrentTC();
	if (tc && tc->isRunning())
	{
		tc->stop(true);
		currentChanged(tc);
	}
}

void KTorrent::removeDownload()
{
	TorrentControl* tc = m_view->getCurrentTC();
	if (tc)
	{
		if (tc->getBytesLeft() > 0 || !tc->isSaved())
		{
			QString msg = i18n("You will lose all data downloaded for this torrent, "
					"if you do this. Are you sure you want to do this?");
			int ret = KMessageBox::warningContinueCancel(this,msg,i18n("Remove Download"),KStdGuiItem::del());
			if (ret == KMessageBox::Cancel)
				return;
		}
		m_info->changeTC(0);
		m_core->remove(tc);
		currentChanged(m_view->getCurrentTC());
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
	KTorrentPreferences dlg(*this);
	dlg.exec();
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

void KTorrent::saveProperties(KConfig *)
{
}

void KTorrent::readProperties(KConfig *)
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
	TorrentControl* tc = m_view->getCurrentTC();
	if (tc)
	{
		bool completed = tc->getBytesLeft() == 0;
		m_save->setEnabled(completed && !tc->isSaved());
		m_start->setEnabled(!tc->isRunning());
		m_stop->setEnabled(tc->isRunning());
		m_remove->setEnabled(true);
	}
	else
	{
		m_save->setEnabled(false);
		m_start->setEnabled(false);
		m_stop->setEnabled(false);
		m_remove->setEnabled(false);
	}
	
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
	m_info->update();
	m_systray_icon->updateStats(tmp + "<br>" + tmp1 + "<br>");
}



#include "ktorrent.moc"
