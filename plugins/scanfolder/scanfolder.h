/***************************************************************************
 *   Copyright (C) 2006 by Ivan Vasić   								   *
 *   ivasic@gmail.com   												   *
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
#ifndef SCANFOLDER_H
#define SCANFOLDER_H

#include <kdirlister.h>
#include <kfileitem.h>
#include <qstring.h>
#include <qobject.h>
#include <qdir.h>
#include <qvaluelist.h>
#include <qtimer.h>
#include <kurl.h>

namespace kt
{
	///Action to perform after loading torrent.
	enum LoadedTorrentAction
	{
	    deleteAction,
	    moveAction,
	    defaultAction
	};
	
	class CoreInterface;

	/**
	 * @brief Scanned folder class.
	 * @author Ivan Vasić <ivasic@gmail.com>
	 * 
	 * It will monitor m_dir directory for changes and automatically pass new torrents to core for loading.
	 * After loading, it will perform specified action which can be:
	 * 1. Deleting torrent in question
	 * 2. Moving torrent to 'loaded' subdirectory
	 * 3. Default action (neither 1. nor 2.)
	 * @see LoadedTorrentAction
	 * 
	*/
	class ScanFolder : public QObject
	{
			Q_OBJECT
		public:
			
			/**
			 * Default constructor.
			 * @param core Pointer to core interface
			 * @param dir Full directory path
			 * @param action Action to perform on loaded torrents.
			 * @param openSilently Wheather to open torrent silently or not.
			 */
			ScanFolder(CoreInterface* core, QString& dir, LoadedTorrentAction action = defaultAction, bool openSilently = true);
			~ScanFolder();

			///Accessor method for m_openSilently.
			bool openSilently() const { return m_openSilently; }
			///Accessor method for m_openSilently
			void setOpenSilently(bool theValue);

			///Accessor method for m_loadedAction.
			void setLoadedAction(const LoadedTorrentAction& theValue);
			///Accessor method for m_loadedAction.
			LoadedTorrentAction loadedAction() const { return m_loadedAction; }

			///Returns true if this object is valid, that is - weather directory is valid and this object does its work.
			bool isValid() const { return m_valid; }
			
			///Sets directory path
			void setFolderUrl(QString& url);

		public slots:
			void onNewItems(const KFileItemList &items);
			void onLoadingFinished(const KURL & url,bool success,bool canceled);
			void onIncompletePollingTimeout();
			
		private:
			/// Check if the URL is a complete file
			bool incomplete(const KURL & src);

		private:
			CoreInterface* m_core;
			
			bool m_valid;
			KDirLister* m_dir;

			LoadedTorrentAction m_loadedAction;
			bool m_openSilently;
			
			QValueList<KURL> m_pendingURLs;
			QValueList<KURL> m_incompleteURLs;

			QTimer m_incomplePollingTimer;
	};
}
#endif
