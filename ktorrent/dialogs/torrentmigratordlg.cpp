/***************************************************************************
 *   Copyright (C) 2008 by Joris Guisson and Ivan Vasic                    *
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
#include <QDir>
#include <klocale.h>
#include <kprogressdialog.h>
#include <kio/copyjob.h>
#include <kmessagebox.h>
#include <kstandardguiitem.h>
#include <interfaces/functions.h>
#include <util/functions.h>
#include <util/fileops.h>
#include <util/log.h>
#include <torrent/torrent.h>
#include <torrent/queuemanager.h>
#include "torrentmigratordlg.h"

using namespace bt;

namespace kt
{

	TorrentMigratorDlg::TorrentMigratorDlg(QWidget* parent)
			: QDialog(parent)
	{
		setupUi(this);
		m_close_button->setGuiItem(KStandardGuiItem::close());
		connect(m_close_button,SIGNAL(clicked()),this,SLOT(accept()));
	}


	TorrentMigratorDlg::~TorrentMigratorDlg()
	{
	}

	bt::Uint32 TorrentMigratorDlg::findTorrentsToBeMigrated()
	{
		QDir h = QDir::home();
		if (!h.cd(".kde/share/apps/ktorrent/"))
		{
			Out(SYS_GEN|LOG_NOTICE) << "No torrents to migrate found" << endl;
			return 0;
		}
		
		QString path = h.absolutePath();
		if (!path.endsWith("/"))
			path += "/";
		
		if (kt::DataDir() == path)
		{
			Out(SYS_GEN|LOG_NOTICE) << "No torrents to migrate found " << endl;
			return 0;
		}
		
		h.setNameFilters(QStringList() << "tor*");
		h.setFilter(QDir::Dirs);
		QStringList sd = h.entryList();
		foreach (const QString & dir,sd)
		{
			bool ok = false;
			Uint32 n = dir.mid(3).toUInt(&ok);
			if (ok)
			{
				todo.insert(h.filePath(dir),n);
				Out(SYS_GEN|LOG_NOTICE) << "Found " << h.filePath(dir) << " to migrate " << n << endl;
			}
		}
		
		return todo.count();
	}
	
	void TorrentMigratorDlg::migrateFoundTorrents(QueueManager* qman)
	{
		if (todo.count() == 0)
			return;
		
		show();
		m_close_button->setEnabled(false);
		
		m_text_output->append(i18np("Importing 1 torrent ...","Importing %1 torrents ...",todo.count()));
	
		m_progress_bar->setRange(0,todo.count());
		m_progress_bar->setValue(0);
		
		Uint32 idx = 0;
		for (QMap<QString,bt::Uint32>::iterator i = todo.begin();i != todo.end();i++)
		{
			m_text_output->append(i18n("Importing <b>%1</b> ...",i.key()));
			doTorrent(i.key(),i.value(),qman);
			m_progress_bar->setValue(++idx);
		}
		
		m_text_output->append(i18n("Finished import."));
		m_progress_bar->setRange(0,100);
		m_progress_bar->setValue(100);
		m_close_button->setEnabled(true);
		exec();
	}
	
	void TorrentMigratorDlg::doTorrent(const QString & tor,bt::Uint32 idx,QueueManager* qman)
	{
		QString path;
		
		// make sure we don't copy over an existing directory 
		do 
		{
			path = kt::DataDir() + QString("tor%1").arg(idx);
			idx++;
		} while (bt::Exists(path));
		
		QString torrent_file = tor;
		if (!torrent_file.endsWith(bt::DirSeparator()))
			torrent_file += bt::DirSeparator();
		torrent_file += "torrent";
		
		// try to load the torrent
		try 
		{
			bt::Torrent t;
			t.load(torrent_file,false);
			// make sure we don't load any dupes
			const SHA1Hash & info_hash = t.getInfoHash();
			for (QueueManager::iterator i = qman->begin();i != qman->end();i++)
			{
				bt::TorrentInterface* ti = *i;
				if (info_hash == ti->getInfoHash())
				{
					m_text_output->append(i18n("Torrent <b>%1</b> already loaded.",tor));
					return;
				}
			}
		}
		catch (...)
		{
			m_text_output->append(i18n("Failed to load <b>%1</b>",torrent_file));
			return;
		}
		
		// everything OK, copy the torX dir
		KIO::Job* j = KIO::copy(KUrl(tor),KUrl(path),KIO::HideProgressInfo);
		if (!j->exec())
		{
			m_text_output->append(i18n("Failed to import <b>%1</b>: %2",tor,j->errorString()));
		}
		else
		{
			m_text_output->append(i18n("Imported <b>%1</b>",tor));
			success << path;
		}
	}
}
