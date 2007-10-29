/***************************************************************************
 *   Copyright (C) 2007 by Joris Guisson                                   *
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
#include <settings.h>
#include <dht/dhtbase.h>
#include <net/socketmonitor.h>
#include <torrent/globals.h>
#include <torrent/server.h>
#include <torrent/choker.h>
#include <torrent/queuemanager.h>
#include <peer/peermanager.h>
#include <util/log.h>
#include <tracker/udptrackersocket.h>
#include <interfaces/coreinterface.h>
#include <interfaces/torrentfileinterface.h>
#include <interfaces/torrentinterface.h>
#include <interfaces/functions.h>
#include "phpcommandhandler.h"

using namespace bt;

namespace kt
{
	bool dht_cmd(const QString & arg)
	{
		if (arg =="start")
		{
			Settings::setDhtSupport(true);
		}
		else
		{
			Settings::setDhtSupport(false);
		}
					
		dht::DHTBase & ht = Globals::instance().getDHT();
		if (Settings::dhtSupport() && !ht.isRunning())
		{
			ht.start(kt::DataDir() + "dht_table",Settings::dhtPort());
			return true;
		}
		else if (!Settings::dhtSupport() && ht.isRunning())
		{
			ht.stop();
			return true;
		}
		else if (Settings::dhtSupport() && ht.getPort() != Settings::dhtPort())
		{
			ht.stop();
			ht.start(kt::DataDir() + "dht_table",Settings::dhtPort());
			return true;
		}
		return false;
	}
	
	bool encryption_cmd(const QString & arg)
	{
		if (arg =="start")
		{
			Settings::setUseEncryption(true);
		}
		else
		{
			Settings::setUseEncryption(false);
		}

		if (Settings::useEncryption())
		{
			Globals::instance().getServer().enableEncryption(Settings::allowUnencryptedConnections());
		}
		else
		{
			Globals::instance().getServer().disableEncryption();
		}
		return true;
	}
	
	PhpCommandHandler::PhpCommandHandler(CoreInterface *c) : core(c)
	{
	}
	
	PhpCommandHandler::~PhpCommandHandler()
	{}
		
	bool PhpCommandHandler::exec(KUrl & url,bool & shutdown)
	{
		const QMap<QString, QString> & params = url.queryItems();
		KUrl redirected_url;
		redirected_url.setPath(url.path());
		
		bool ret = false;
		shutdown = false;
		QMap<QString, QString>::ConstIterator it;
		for ( it = params.begin(); it != params.end(); ++it ) 
		{
		//	Out(SYS_WEB| LOG_DEBUG) << "exec " << it.key().latin1() << endl;
			if(it.key()=="dht")
			{
				if (dht_cmd(it.value()))
					ret = true;
			}
			else if(it.key()=="encription")
			{
				if (encryption_cmd(it.value()))
					ret = true;
			}
			else if(it.key()=="global_connection")
			{
				Settings::setMaxTotalConnections(it.value().toInt());
				PeerManager::setMaxTotalConnections(Settings::maxTotalConnections());
				ret = true;
			}
			else if(it.key()=="load_torrent" && it.value().length() > 0)
			{
				core->loadSilently(QUrl::fromPercentEncoding(it.value().toLocal8Bit()));
				ret = true;
			}
			if(it.key()=="maximum_downloads")
			{
				int max = it.value().toInt();
				core->getQueueManager()->setMaxDownloads(max);
				Settings::setMaxDownloads(max);
				ret = true;
			}
			else if(it.key()=="maximum_seeds")
			{
				core->getQueueManager()->setMaxSeeds(it.value().toInt());
				Settings::setMaxSeeds(it.value().toInt());	
				ret = true;
			}
			else if(it.key()=="maximum_connection_per_torrent")
			{
				PeerManager::setMaxConnections(it.value().toInt());
				Settings::setMaxConnections(it.value().toInt());
				ret = true;
			}
			else if(it.key()=="maximum_upload_rate")
			{
				Settings::setMaxUploadRate(it.value().toInt());
				net::SocketMonitor::setUploadCap( Settings::maxUploadRate() * 1024);
				ret = true;
			}
			else if(it.key()=="maximum_download_rate")
			{
				Settings::setMaxDownloadRate(it.value().toInt());
				net::SocketMonitor::setDownloadCap(Settings::maxDownloadRate()*1024);
				ret = true;
			}
			else if(it.key()=="maximum_share_ratio")
			{
				Settings::setMaxRatio(it.value().toInt());
				ret = true;
			}
			else if(it.key()=="number_of_upload_slots")
			{
				Settings::setNumUploadSlots(it.value().toInt());
				Choker::setNumUploadSlots(Settings::numUploadSlots());
				ret = true;
			}
			else if(it.key()=="port")
			{
				Settings::setPort(it.value().toInt());
				core->changePort(Settings::port());
			}
			else if(it.key()=="port_udp_tracker")
			{
				Settings::setUdpTrackerPort(it.value().toInt());
				UDPTrackerSocket::setPort(Settings::udpTrackerPort());
				ret = true;
			}
			else if(it.key()=="quit" && !it.value().isEmpty())
			{
				shutdown = true;
				ret = true;
			}
			else if (it.key()=="remove")
			{
				QList<TorrentInterface*>::iterator i= core->getQueueManager()->begin();
				for(int k=0; i != core->getQueueManager()->end(); i++, k++)
				{
					if(it.value().toInt()==k)
					{
						core->remove((*i), false);
						ret = true;
						break;
					}
				}
			}
			else if(it.key()=="stopall" && !it.value().isEmpty())
			{
				core->stopAll(3);
				ret = true;
			}
			else if(it.key()=="startall" && !it.value().isEmpty())
			{
				core->startAll(3);
				ret = true;
			}
			else if(it.key()=="stop")
			{
				QList<TorrentInterface*>::iterator i= core->getQueueManager()->begin();
				for(int k=0; i != core->getQueueManager()->end(); i++, k++)
				{
					if(it.value().toInt()==k)
					{
						(*i)->stop(true);
						ret = true;
						break;
					}
				}
			}
			else if(it.key()=="start")
			{
				QList<TorrentInterface*>::iterator i= core->getQueueManager()->begin();
				for(int k=0; i != core->getQueueManager()->end(); i++, k++)
				{
					if(it.value().toInt()==k)
					{
						(*i)->start();
						ret = true;
						break;
					}
				}
			}
			else if (it.key().startsWith("file_"))
			{
				QString torrent_num;
				QString file_num;
				//parse argument into torrent number and file number
				int separator_loc=it.value().indexOf('-');
				QString parse = it.value();
				
				torrent_num.append(parse.left(separator_loc));
				file_num.append(parse.right(parse.length()-(separator_loc+1)));

				if(it.key()=="file_lp")
				{
					QList<TorrentInterface*>::iterator i= core->getQueueManager()->begin();
					for(int k=0; i != core->getQueueManager()->end(); i++, k++)
					{
						if(torrent_num.toInt()==k)
						{
							TorrentFileInterface & file = (*i)->getTorrentFile(file_num.toInt());
							file.setPriority(LAST_PRIORITY);
							break;
						}
					}
				}
				else if(it.key()=="file_np")
				{
					QList<TorrentInterface*>::iterator i= core->getQueueManager()->begin();
					for(int k=0; i != core->getQueueManager()->end(); i++, k++)
					{
						if(torrent_num.toInt()==k)
						{
							TorrentFileInterface & file = (*i)->getTorrentFile(file_num.toInt());
							file.setPriority(NORMAL_PRIORITY);
							break;
						}
					}
				}
				else if(it.key()=="file_hp")
				{
					QList<TorrentInterface*>::iterator i= core->getQueueManager()->begin();
					for(int k=0; i != core->getQueueManager()->end(); i++, k++)
					{
						if(torrent_num.toInt()==k)
						{
							TorrentFileInterface & file = (*i)->getTorrentFile(file_num.toInt());
							file.setPriority(FIRST_PRIORITY);
							break;
						}
					}
				}
				else if(it.key()=="file_dnd")
				{
					QList<TorrentInterface*>::iterator i= core->getQueueManager()->begin();
					for(int k=0; i != core->getQueueManager()->end(); i++, k++)
					{
						if(torrent_num.toInt()==k)
						{
							TorrentFileInterface & file = (*i)->getTorrentFile(file_num.toInt());
							file.setPriority(ONLY_SEED_PRIORITY);
							break;
						}
					}
				}
				else
				{
				// add unknown query items to the redirected url
				// we don't add the keys above, because if the user presses refresh 
				// the same action will be taken again
					redirected_url.addQueryItem(it.key(),it.value());
				}
			}
			else
			{
				// add unknown query items to the redirected url
				// we don't add the keys above, because if the user presses refresh 
				// the same action will be taken again
				redirected_url.addQueryItem(it.key(),it.value());
			}
		}
		
		if (ret)
			url = redirected_url; 
		
		return ret;
	}
}
