/***************************************************************************
 *   Copyright (C) 2010 by Joris Guisson                                   *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/

#ifndef BT_PEERCONNECTOR_H
#define BT_PEERCONNECTOR_H

#include <QSet>
#include <btcore_export.h>
#include <util/constants.h>


namespace bt
{
	class Authenticate;
	class PeerManager;

	/**
		Class which connects to a peer.
	*/
	class BTCORE_EXPORT PeerConnector : public QObject
	{
		Q_OBJECT
	public:
		enum Method
		{
			TCP_WITH_ENCRYPTION,
			TCP_WITHOUT_ENCRYPTION,
			UTP_WITH_ENCRYPTION,
			UTP_WITHOUT_ENCRYPTION
		};
		
		PeerConnector(const QString & ip,Uint16 port,bool local,PeerManager* pman);
		virtual ~PeerConnector();
	
		/// Called when an authentication attempt is finished
		void authenticationFinished(bt::Authenticate* auth, bool ok);
		
	private:
		void start(Method method);
		
	private:
		QSet<Method> tried_methods;
		Method current_method;
		QString ip;
		Uint16 port;
		bool local;
		PeerManager* pman;
		Authenticate* auth;
		bool stopping;
	};

}

#endif // BT_PEERCONNECTOR_H
