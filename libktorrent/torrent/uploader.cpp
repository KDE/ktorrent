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
#include <util/log.h>
#include "uploader.h"
#include "peer.h"
#include "chunkmanager.h"
#include "request.h"
#include "uploader.h"
#include "peeruploader.h"
#include "peermanager.h"


namespace bt
{

	Uploader::Uploader(ChunkManager & cman,PeerManager & pman) 
	: cman(cman),pman(pman),uploaded(0)
	{}


	Uploader::~Uploader()
	{
	}

	
		
	void Uploader::update(Uint32 opt_unchoked)
	{
		for (Uint32 i = 0;i < pman.getNumConnectedPeers();++i)
		{
			PeerUploader* p = pman.getPeer(i)->getPeerUploader();
			uploaded += p->update(cman,opt_unchoked);
		}
	}
	

	Uint32 Uploader::uploadRate() const
	{
		Uint32 rate = 0;
		for (Uint32 i = 0;i < pman.getNumConnectedPeers();++i)
		{
			const Peer* p = pman.getPeer(i);
			rate += p->getUploadRate();
		}
		return rate;
	}
	

}
#include "uploader.moc"
