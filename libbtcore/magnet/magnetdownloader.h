/***************************************************************************
 *   Copyright (C) 2009 by Joris Guisson                                   *
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

#ifndef BT_MAGNETDOWNLOADER_H
#define BT_MAGNETDOWNLOADER_H

#include <QObject>
#include <btcore_export.h>
#include <torrent/torrent.h>
#include <tracker/tracker.h>
#include "magnetlink.h"


namespace dht
{
	class DHTPeerSource;
}


namespace bt
{
	class Peer;
	class PeerManager;
	
	/**
		Class which tries to download the metadata associated to a MagnetLink
		It basically has a Tracker (optional), a DHTPeerSource and a PeerManager.
		With these it tries to find peers, connect to them and download the metadata.
	*/
	class BTCORE_EXPORT MagnetDownloader : public QObject, public TrackerDataSource
	{
		Q_OBJECT
	public:
		MagnetDownloader(const MagnetLink & mlink,QObject* parent);
		virtual ~MagnetDownloader();
		
		/**
			Update the MagnetDownloader
		*/
		void update();
		
		/// Is the magnet download running
		bool running() const;
		
		/// How many peers are we connected to
		Uint32 numPeers() const;
		
		/// Get the MagnetLink
		const MagnetLink & magnetLink() const {return mlink;}
		
	public slots:
		/**
		Start the MagnetDownloader, this will enable DHT.
		*/
		void start();
		
		/**
		Stop the MagnetDownloader
		*/
		void stop();
		
	signals:
		/**
			Emitted when downloading the metadata was succesfull.
		*/
		void foundMetadata(bt::MagnetDownloader* self,const QByteArray & metadata);
		
	private slots:
		void onNewPeer(Peer* p);
		void onMetadataDownloaded(const QByteArray & data);
		void dhtStarted();
		void dhtStopped();
		
	private:
		virtual Uint64 bytesDownloaded() const;
		virtual Uint64 bytesUploaded() const;
		virtual Uint64 bytesLeft() const;
		virtual const SHA1Hash & infoHash() const;
		
	private:
		MagnetLink mlink;
		Tracker* tracker;
		PeerManager* pman;
		dht::DHTPeerSource* dht_ps;
		QByteArray metadata;
		Torrent tor;
		bool found;
	};

}

#endif // BT_MAGNETDOWNLOADER_H
