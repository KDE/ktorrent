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
#include "log.h"
#include "uploader.h"
#include "peer.h"
#include "chunkmanager.h"
#include "request.h"
#include "uploader.h"
#include "peeruploader.h"


namespace bt
{

	Uploader::Uploader(ChunkManager & cman) 
	: cman(cman),uploaded(0)
	{}


	Uploader::~Uploader()
	{
		for (UpItr i = uploaders.begin();i != uploaders.end();i++)
		{
			PeerUploader* p = i->second;
			delete p;
		}
	}

	void Uploader::addRequest(const Request & req)
	{
		UpItr i = uploaders.find(req.getPeer());
		if (i != uploaders.end())
		{
			PeerUploader* p = i->second;
			p->addRequest(req);
		}
	}
	
	void Uploader::cancel(const Request & req)
	{
		UpItr i = uploaders.find(req.getPeer());
		if (i != uploaders.end())
		{
			PeerUploader* p = i->second;
			p->removeRequest(req);
		}
	}
	
	void Uploader::addPeer(Peer* peer)
	{
		PeerUploader* pup = new PeerUploader(peer,cman);
		uploaders[peer] = pup;
	}
	
	void Uploader::removePeer(Peer* peer)
	{
		UpItr i = uploaders.find(peer);
		if (i != uploaders.end())
		{
			delete i->second;
			uploaders.erase(i);
		}
	}
	
	void Uploader::removeAllPeers()
	{
		for (UpItr i = uploaders.begin();i != uploaders.end();i++)
		{
			PeerUploader* p = i->second;
			delete p;
		}
		uploaders.clear();
	}
	
	void Uploader::update()
	{
		for (UpItr i = uploaders.begin();i != uploaders.end();i++)
		{
			PeerUploader* p = i->second;
			uploaded += p->update();
		}
	}
	

	Uint32 Uploader::uploadRate() const
	{
		Uint32 rate = 0;
		UpCItr j = uploaders.begin();
		while (j != uploaders.end())
		{
			const Peer* p = j->first;
			rate += p->getUploadRate();
			j++;
		}
		return rate;
	}
	

}
#include "uploader.moc"
