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

#include "peerconnector.h"
#include <interfaces/serverinterface.h>
#include <mse/encryptedauthenticate.h>
#include <torrent/torrent.h>
#include "peermanager.h"
#include "authenticationmonitor.h"


namespace bt
{
	
	PeerConnector::PeerConnector(const QString & ip,Uint16 port,bool local,PeerManager* pman) 
		: QObject(pman),ip(ip),port(port),local(local),pman(pman),auth(0),stopping(false)
	{
		bool encryption = ServerInterface::isEncryptionEnabled();
		bool utp = ServerInterface::isUtpEnabled();
		
		if (encryption)
		{
			if (utp)
				start(UTP_WITH_ENCRYPTION);
			else
				start(TCP_WITH_ENCRYPTION);
		}
		else
		{
			if (utp)
				start(UTP_WITHOUT_ENCRYPTION);
			else
				start(TCP_WITHOUT_ENCRYPTION);
		}
	}

	PeerConnector::~PeerConnector()
	{
		if (auth)
		{
			stopping = true;
			auth->stop();
			stopping = false;
		}
	}

	void PeerConnector::authenticationFinished(Authenticate* auth, bool ok)
	{
		this->auth = 0;
		if (stopping)
			return;
		
		if (ok)
		{
			pman->peerAuthenticated(auth,this,ok);
			return;
		}
		
		tried_methods.insert(current_method);
		
		QList<Method> allowed;
		
		bool encryption = ServerInterface::isEncryptionEnabled();
		bool only_use_encryption = !ServerInterface::unencryptedConnectionsAllowed();
		bool utp = ServerInterface::isUtpEnabled();
		bool only_use_utp = ServerInterface::onlyUseUtp();
		
		if (utp && encryption)
			allowed.append(UTP_WITH_ENCRYPTION);
		if (!only_use_utp && encryption)
			allowed.append(TCP_WITH_ENCRYPTION);
		if (utp && !only_use_encryption)
			allowed.append(UTP_WITHOUT_ENCRYPTION);
		if (!only_use_utp && !only_use_encryption)
			allowed.append(TCP_WITHOUT_ENCRYPTION);
		
		foreach (Method m,tried_methods)
			allowed.removeAll(m);
		
		if (allowed.isEmpty())
			pman->peerAuthenticated(auth,this,false);
		else
			start(allowed.front());
	}

	void PeerConnector::start(PeerConnector::Method method)
	{
		current_method = method;
		Torrent & tor = pman->getTorrent();
		TransportProtocol proto = (method == TCP_WITH_ENCRYPTION || method == TCP_WITHOUT_ENCRYPTION) ? TCP : UTP;
		if (method == TCP_WITH_ENCRYPTION || method == UTP_WITH_ENCRYPTION)
			auth = new mse::EncryptedAuthenticate(ip,port,proto,tor.getInfoHash(),tor.getPeerID(),this);
		else
			auth = new Authenticate(ip,port,proto,tor.getInfoHash(),tor.getPeerID(),this);
		
		if (local)
			auth->setLocal(true);
		
		connect(pman,SIGNAL(stopped()),auth,SLOT(stop()));
		AuthenticationMonitor::instance().add(auth);
	}

}


