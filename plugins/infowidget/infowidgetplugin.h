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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#ifndef KTINFOWIDGETPLUGIN_H
#define KTINFOWIDGETPLUGIN_H

#include <interfaces/plugin.h>
#include <interfaces/guiinterface.h>



namespace kt
{
	class PeerView;
	class TrackerView;
	class StatusTab;
	class FileView;
	class ChunkDownloadView;
	class InfoWidgetPrefPage;
	class KTorrentMonitor;
	

	/**
	@author Joris Guisson
	*/
	class InfoWidgetPlugin : public Plugin,public ViewListener
	{
		Q_OBJECT
	public:
		InfoWidgetPlugin(QObject* parent, const char* name, const QStringList& args);
		virtual ~InfoWidgetPlugin();

		virtual void load();
		virtual void unload();
		virtual void guiUpdate();
		virtual void currentTorrentChanged(TorrentInterface* tc);
		virtual bool versionCheck(const QString & version) const;
		
		///Show PeerView in main window
		void showPeerView(bool show);
		///Show ChunkDownloadView in main window
		void showChunkView(bool show);
		///Show TrackerView in main window
		void showTrackerView(bool show);
	private:
		void createMonitor(TorrentInterface* tc);
		
	private:
		PeerView* peer_view;
		ChunkDownloadView* cd_view;
		TrackerView* tracker_view;
		FileView* file_view;
		StatusTab* status_tab;
		KTorrentMonitor* monitor;
		
		InfoWidgetPrefPage* pref;
	};

}

#endif
