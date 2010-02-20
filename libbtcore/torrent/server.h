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
#ifndef BTSERVER_H
#define BTSERVER_H

#include <qlist.h>
#include <qobject.h>
#include <btcore_export.h>
#include <interfaces/serverinterface.h>
#include "globals.h"


class QSocketNotifier;

namespace net
{
	class Socket;
}

namespace bt
{
	class PeerManager;
	class SHA1Hash;


	/**
	 * @author Joris Guisson
	 *
	 * Class which listens for incoming connections.
	 * Handles authentication and then hands of the new
	 * connections to a PeerManager.
	 *
	 * All PeerManager's should register with this class when they
	 * are created and should unregister when they are destroyed.
	 */
	class BTCORE_EXPORT Server : public ServerInterface
	{
		Q_OBJECT

		net::Socket* sock;
		QSocketNotifier* sn;

	public:
		Server();
		virtual ~Server();
		
		virtual bool changePort(Uint16 port);
		
		void close();

	private slots:
		void readyToAccept(int fd);
	};

}

#endif
