/***************************************************************************
 *   Copyright (C) 2007 by Jaak Ristioja                                   *
 *   Ristioja@gmail.com                                                    *
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
#include <kiconloader.h>
#include <kmimetype.h>
#include <interfaces/functions.h>
#include <interfaces/torrentinterface.h>
#include <interfaces/torrentfileinterface.h>
#include "iwfiletreeitem.h"
#include "iwfiletreediritem.h"
#include "fileview.h"
#include "fileviewupdatethread.h"

namespace kt
{
	FileViewUpdateThread::FileViewUpdateThread(FileView* fileview)
		: fileview(fileview), stopThread(true)
	{
		// empty
	}
	
	FileViewUpdateThread::~FileViewUpdateThread()
	{
		// empty
	}
	
	void FileViewUpdateThread::start(kt::TorrentInterface* new_tc, QThread::Priority priority) {
		stopThread = false;
		this->new_tc = new_tc;
		QThread::start(priority);
	}
	
	void FileViewUpdateThread::run()
	{
		fileview->setEnabled(false);
		
		if (!fillFileTree()) return;
		
		if (new_tc != 0) {
			QObject::connect(
					new_tc,
	 				SIGNAL(missingFilesMarkedDND( kt::TorrentInterface* )),
					fileview,
	 				SLOT(refreshFileTree( kt::TorrentInterface* ))
				);
			fileview->setEnabled(true);
		}
	}
	
	void FileViewUpdateThread::stop()
	{
		stopThread = true;
	}
	
	bool FileViewUpdateThread::fillFileTree()
	{
		fileview->eventlock.lock();
		fileview->curr_tc = new_tc;
		fileview->multi_root = 0;
		fileview->clear();
		fileview->eventlock.unlock();
		
		if (!fileview->curr_tc)
			return true;
		
		if (fileview->curr_tc->getStats().multi_file_torrent)
		{
			fileview->eventlock.lock();
			IWFileTreeDirItem* root = new IWFileTreeDirItem(
					fileview,
					fileview->curr_tc->getStats().torrent_name
				);
			fileview->eventlock.unlock();
			
			for (Uint32 i = 0; i < fileview->curr_tc->getNumFiles(); i++)
			{
				TorrentFileInterface & file = fileview->curr_tc->getTorrentFile(i);
				
				fileview->eventlock.lock();
				root->insert(file.getPath(), file);
				if (stopThread) {
					fileview->clear();
					fileview->eventlock.unlock();
					return false;
				}
				fileview->eventlock.unlock();
			}
			fileview->eventlock.lock();
			root->setOpen(true);
			fileview->setRootIsDecorated(true);
			fileview->multi_root = root;
			fileview->multi_root->updatePriorityInformation(fileview->curr_tc);
			fileview->multi_root->updatePercentageInformation();
			fileview->multi_root->updatePreviewInformation(fileview->curr_tc);
			fileview->eventlock.unlock();
		}
		else
		{
			const TorrentStats & s = fileview->curr_tc->getStats();
			fileview->eventlock.lock();
			fileview->setRootIsDecorated(false);
			KListViewItem* item = new KListViewItem(
					fileview,
					s.torrent_name,
					BytesToString(s.total_bytes)
				);
			
			item->setPixmap(0, KMimeType::findByPath(s.torrent_name)->pixmap(KIcon::Small));
			fileview->eventlock.unlock();
		}
		return true;
	}
} // namespace kt
