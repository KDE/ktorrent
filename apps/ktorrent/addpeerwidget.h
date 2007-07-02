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
#ifndef ADDPEERWIDGET_H
#define ADDPEERWIDGET_H

#include "addpeerwidgetbase.h"

#include <qobject.h>
#include <interfaces/peersource.h>

namespace kt
{
	class TorrentInterface;
}

namespace bt
{
	class WaitJob;
}

/**
 * @author Ivan Vasic <ivasic@gmail.com>
 * @brief PeerSource to allow peers to be added manually.
 * 
 * Start/Stop() do nothing. Use this class only to manually add peers.
 */
class ManualPeerSource: public kt::PeerSource
{
	Q_OBJECT
			
	public:
		ManualPeerSource();
		virtual ~ManualPeerSource();
		
		/**
		 * @brief Emits peersReady signal.
		 * Use this when there are peers available in this peer source.
		 */
		void signalPeersReady();
		
	public slots:
		
	/**
	 * Start gathering peers.
	 * Had to be defined but won't be used here.
	 */
	void start();
		
	/**
	 * Stop gathering peers.
	 * Had to be defined but won't be used here.
	*/
	void stop(bt::WaitJob* wjob = 0);
};


/**
 * @author Ivan Vasic <ivasic@gmail.com>
 * @brief GUI dialog for adding peers.
 */
class AddPeerWidget: public AddPeerWidgetBase
{
		Q_OBJECT
	public:
		AddPeerWidget(kt::TorrentInterface* tc, QWidget *parent = 0, const char *name = 0);
		~AddPeerWidget();
		
	public slots:
		virtual void btnAdd_clicked();
		
	private:
		ManualPeerSource* m_peerSource;
		kt::TorrentInterface* m_tc;
};

#endif
