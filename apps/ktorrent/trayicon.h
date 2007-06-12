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
#ifndef TRAYICON_H
#define TRAYICON_H


#include <ksystemtray.h>
#include <kpopupmenu.h>
#include <qpainter.h>

#include "ktorrentcore.h"
#include "settings.h"
#include "interfaces/torrentinterface.h"
#include <util/constants.h>

using namespace bt;
class QString;
class TrayHoverPopup;

struct TrayStats
{
	bt::Uint32 download_speed;
	bt::Uint32 upload_speed;
	bt::Uint64 bytes_downloaded;
	bt::Uint64 bytes_uploaded;
};

/**
 * @author Joris Guisson
 * @author Ivan Vasic
*/
class TrayIcon : public KSystemTray
{
	Q_OBJECT
public:
	TrayIcon(KTorrentCore* tc, QWidget *parent = 0, const char *name = 0);
	virtual ~TrayIcon();

	/// Update stats for system tray icon
	void updateStats(const CurrentStats stats, bool showBars=false, int downloadBandwidth=0, int uploadBandwidth=0);
	
private:
	void drawSpeedBar(int downloadSpeed, int uploadSpeed, int downloadBandwidth, int uploadBandwidth);
	void showPassivePopup(const QString & msg,const QString & titile);
	virtual void enterEvent(QEvent* ev);
	virtual void leaveEvent(QEvent* ev);

private slots:
	/**
	 * Show a passive popup, that the torrent has stopped downloading.
	 * @param tc The torrent
	 */
	void finished(kt::TorrentInterface* tc);
	
	/**
	 * Show a passive popup that a torrent has reached it's max share ratio. 
	 * @param tc The torrent
	 */
	void maxShareRatioReached(kt::TorrentInterface* tc);
	
	/**
	 * Show a passive popup that a torrent has reached it's max seed time
	 * @param tc The torrent
	 */
	void maxSeedTimeReached(kt::TorrentInterface* tc);
	
	/**
	 * Show a passive popup when a torrent has been stopped by an error.
	 * @param tc The torrent 
	 * @param msg Error message
	 */
	void torrentStoppedByError(kt::TorrentInterface* tc, QString msg);
	
	/**
	 * Corrupted data has been found.
	 * @param tc The torrent
	 */
	void corruptedData(kt::TorrentInterface* tc);
	
	/**
	 * User tried to enqueue a torrent that has reached max share ratio or max seed time
	 * Show passive popup message.
	 */
	void queuingNotPossible(kt::TorrentInterface* tc);
	
	/**
	 * We failed to start a torrent
	 * @param tc The torrent
	 * @param reason The reason it failed
	 */
	void canNotStart(kt::TorrentInterface* tc,kt::TorrentStartResponse reason);
	
	///Shows passive popup message
	void lowDiskSpace(kt::TorrentInterface* tc, bool stopped);
	
private:
	KTorrentCore* m_core;
	QPainter *paint;
	int previousDownloadHeight;
	int previousUploadHeight;
	TrayHoverPopup* m_hover_popup;
	QPixmap m_kt_pix;
};

class SetMaxRate : public KPopupMenu
{
		Q_OBJECT
	public:
		SetMaxRate(KTorrentCore* tc, int t, QWidget *parent=0, const char *name=0); // type: 0 Upload; 1 Download
		~SetMaxRate()
		{}
		;

		void update();
	private:
		KTorrentCore* m_core;
		int type;
		void makeMenu();
	private slots:
		void rateSelected(int id);
};

#endif
