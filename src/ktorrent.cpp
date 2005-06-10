/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson                                   *
 *   joris.guisson@gmail.com                                               *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/



#include <qdragobject.h>
#include <qsplitter.h>

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

#include <kstdaccel.h>
#include <kaction.h>
#include <kstdaction.h>


#include <libtorrent/torrentcontrol.h>
#include <libtorrent/peermanager.h>
#include <libtorrent/uploadcap.h>
#include <libtorrent/error.h>
#include <libtorrent/globals.h>
#include <libtorrent/log.h>

#include "ktorrentcore.h"
#include "ktorrentview.h"
#include "ktorrent.h"
#include "pref.h"
#include "settings.h"
#include "trayicon.h"
#include "searchwidget.h"
#include "logviewer.h"




using namespace bt;

KTorrent::KTorrent()
		: KMainWindow( 0, "KTorrent" ),
		m_view(0),m_systray_icon(0)
{
	bool debug = bt::Globals::instance().isDebugModeSet();
	QSplitter* s;
	if (debug)
		s = new QSplitter(QSplitter::Vertical,this);
	
	m_tabs = new KTabWidget(debug ? (QWidget*)s : (QWidget*)this);
	m_view = new KTorrentView(m_tabs);
	m_search = new SearchWidget(m_tabs);
	m_core = new KTorrentCore();
	m_systray_icon = new TrayIcon(this);

	m_tabs->addTab(m_view,i18n("Downloads"));
	m_tabs->addTab(m_search,i18n("Search"));	
	
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
		
	// accept dnd
	setAcceptDrops(true);



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
	
	m_core->loadTorrents();

	connect(m_search,SIGNAL(statusBarMsg(const QString& )),this,SLOT(changeStatusbar(const QString& )));
	connect(m_search,SIGNAL(openTorrent(const KURL& )),this,SLOT(load(const KURL& )));
}

KTorrent::~KTorrent()
{
	bt::Out() << "I'm dead" << bt::endl;
	delete m_core;
//	delete m_mon;
}

void KTorrent::applySettings()
{
	m_core->setMaxDownloads(Settings::maxDownloads());
	PeerManager::setMaxConnections(Settings::maxConnections());
	UploadCap::setSpeed(Settings::maxUploadRate() * 1024);
	TorrentControl::setInitialPort(Settings::port());
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
}

void KTorrent::load(const KURL& url)
{
	QString target;
	// the below code is what you should normally do.  in this
	// example case, we want the url to our own.  you probably
	// want to use this code instead for your app


	// download the contents
	if (KIO::NetAccess::download(url,target,this))
	{
		// set our caption
		setCaption(url.prettyURL());

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
		m_save->setEnabled(completed);
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
}

void KTorrent::setupActions()
{
	KStdAction::open(this, SLOT(fileOpen()), actionCollection());
	KStdAction::quit(kapp, SLOT(quit()), actionCollection());
	m_save = KStdAction::save(this, SLOT(fileSave()), actionCollection());

	m_toolbarAction = KStdAction::showToolbar(this, SLOT(optionsShowToolbar()), actionCollection());
	m_statusbarAction = KStdAction::showStatusbar(this, SLOT(optionsShowStatusbar()), actionCollection());

	KStdAction::keyBindings(this, SLOT(optionsConfigureKeys()), actionCollection());
	KStdAction::configureToolbars(this, SLOT(optionsConfigureToolbars()), actionCollection());
	
	KAction* pref = KStdAction::preferences(this, SLOT(optionsPreferences()), actionCollection());

	pref->plug(m_systray_icon->contextMenu(),1);

	
	m_start = new KAction(
			i18n("Start"), "player_play",0,this, SLOT(startDownload()),
			actionCollection(), "Start");
	
	m_stop = new KAction(
			i18n("Stop"), "player_stop",0,this, SLOT(stopDownload()),
			actionCollection(), "Stop");
	
	m_remove = new KAction(
			i18n("Remove"), "remove",0,this, SLOT(removeDownload()),
			actionCollection(), "Remove");
	
	createGUI();
}

void KTorrent::saveProperties(KConfig *config)
{
	// the 'config' object points to the session managed
	// config file.  anything you write here will be available
	// later when this app is restored

/*	if (!m_view->currentURL().isEmpty())
	{
#if KDE_IS_VERSION(3,1,3)
		config->writePathEntry("lastURL", m_view->currentURL());
#else
		config->writeEntry("lastURL", m_view->currentURL());
#endif

}*/
}

void KTorrent::readProperties(KConfig *config)
{
	// the 'config' object points to the session managed
	// config file.  this function is automatically called whenever
	// the app is being restored.  read in here whatever you wrote
	// in 'saveProperties'

	/*QString url = config->readPathEntry("lastURL");

	if (!url.isEmpty())
	m_view->openURL(KURL(url));*/
}

void KTorrent::dragEnterEvent(QDragEnterEvent *event)
{
	// accept uri drops only
	event->accept(KURLDrag::canDecode(event));
}

void KTorrent::dropEvent(QDropEvent *event)
{
	// this is a very simplistic implementation of a drop event.  we
	// will only accept a dropped URL.  the Qt dnd code can do *much*
	// much more, so please read the docs there
	KURL::List urls;

	// see if we can decode a URI.. if not, just ignore it
	if (KURLDrag::decode(event, urls) && !urls.isEmpty())
	{
		// okay, we have a URI.. process it
		const KURL &url = urls.first();

		// load in the file
		load(url);
	}
}

void KTorrent::fileOpen()
{
	// this slot is called whenever the File->Open menu is selected,
	// the Open shortcut is pressed (usually CTRL+O) or the Open toolbar
	// button is clicked
	/*
	    // this brings up the generic open dialog
	    KURL url = KURLRequesterDlg::getURL(QString::null, this, i18n("Open Location") );
	*/
	// standard filedialog
	KURL url = KFileDialog::getOpenURL(QString::null, QString::null, this, i18n("Open Location"));

	if (url.isValid())
		load(url);
}

void KTorrent::save(bt::TorrentControl* tc)
{
	if (!tc || tc->getBytesLeft() != 0)
		return;
	
	KProgressDialog* dlg = 0;
	
	try
	{
		QString file = QString::null;
		if (tc->isMultiFileTorrent())
		{
			file = KFileDialog::getExistingDirectory(QString::null, this,
					i18n("Select Directory to Save"));
		}
		else
		{
			file = KFileDialog::getSaveFileName(
					QString::null, QString::null, this, i18n("Select File to Save"));
		}

		if (file != QString::null)
		{
			if (tc->isMultiFileTorrent())
			{
				dlg = new KProgressDialog(this);
				dlg->setLabel(i18n("Saving torrent ..."));
				dlg->setAllowCancel(false);
				dlg->show();
			}
			tc->reconstruct(file,dlg);
		}
	}
	catch (bt::Error & err)
	{
		KMessageBox::error(0,err.toString(),"Error");
	}
	
	delete dlg;
}

void KTorrent::askAndSave(bt::TorrentControl* tc)
{
	int ret = KMessageBox::questionYesNo(
		this,
		i18n("The download %1 has finished. Do you want to save it now ?")
				.arg(tc->getTorrentName()),
		i18n("Save Torrent?"));
	
	if (ret == KMessageBox::Yes)
		save(tc);
}

void KTorrent::fileSave()
{
	TorrentControl* tc = m_view->getCurrentTC();
	save(tc);
}

void KTorrent::startDownload()
{
	TorrentControl* tc = m_view->getCurrentTC();
	if (tc && tc->getBytesLeft() != 0 && !tc->isRunning())
	{
		tc->start();
		currentChanged(tc);
	}
}

void KTorrent::stopDownload()
{
	TorrentControl* tc = m_view->getCurrentTC();
	if (tc && tc->isRunning())
	{
		tc->stop();
		currentChanged(tc);
	}
}

void KTorrent::removeDownload()
{
	TorrentControl* tc = m_view->getCurrentTC();
	if (tc)
	{
		m_core->remove(tc);
	}
}

void KTorrent::optionsShowToolbar()
{
	// this is all very cut and paste code for showing/hiding the
	// toolbar
	if (m_toolbarAction->isChecked())
		toolBar()->show();
	else
		toolBar()->hide();
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
	statusBar()->message(text);
}

void KTorrent::changeCaption(const QString& text)
{
	// display the text on the caption
	setCaption(text);
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

#include "ktorrent.moc"
