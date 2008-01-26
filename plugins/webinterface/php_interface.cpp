  /***************************************************************************
 *   Copyright (C) 2006 by Diego R. Brogna                                 *
 *   dierbro@gmail.com                                               	   *
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

#include <kio/global.h>
#include <kglobal.h>
#include <klocale.h>

#include <net/socketmonitor.h>
#include <torrent/choker.h>
#include <torrent/udptrackersocket.h>
#include <kademlia/dhtbase.h>
#include <torrent/server.h>
#include <util/log.h>
#include <interfaces/functions.h>

#include "php_interface.h"
		
using namespace bt;

namespace kt
{
	extern QString DataDir();
	
	using bt::FIRST_PRIORITY;
 	using bt::NORMAL_PRIORITY;
 	using bt::LAST_PRIORITY;
 	using bt::EXCLUDED;

	
	QString BytesToString2(Uint64 bytes,int precision = 2)
	{
		KLocale* loc = KGlobal::locale();
		if (bytes >= 1024 * 1024 * 1024)
			return QString("%1 GB").arg(loc->formatNumber(bytes / TO_GIG,precision < 0 ? 2 : precision));
		else if (bytes >= 1024*1024)
			return QString("%1 MB").arg(loc->formatNumber(bytes / TO_MEG,precision < 0 ? 1 : precision));
		else if (bytes >= 1024)
			return QString("%1 KB").arg(loc->formatNumber(bytes / TO_KB,precision < 0 ? 1 : precision));
		else
			return QString("%1 B").arg(bytes);
	}

	QString KBytesPerSecToString2(double speed,int precision = 2)
	{
		KLocale* loc = KGlobal::locale();
		return QString("%1 KB/s").arg(loc->formatNumber(speed,precision));
	}

	/************************
	*PhpCodeGenerator	*
	************************/
	PhpCodeGenerator::PhpCodeGenerator(CoreInterface *c)
	{
		core=c;
	}
	
	/*Generate php code
	* function downloadStatus()
	* {
	*	return array( ... );
	* }
	*/
	void PhpCodeGenerator::downloadStatus(QTextStream & out)
	{
		TorrentStats stats;
		//Priority file_priority;
		QString status;
		out << "function downloadStatus()\n{\nreturn array(";

		QPtrList<TorrentInterface>::iterator i= core->getQueueManager()->begin();
		for(int k=0; i != core->getQueueManager()->end(); i++, k++)
		{
			if (k > 0)
				out << ",\n";
			
			stats=(*i)->getStats();
			out << QString("\n%1 => array(").arg(k);
			
			out << QString("\"imported_bytes\" => %1,\n").arg(stats.imported_bytes);
			out << QString("\"bytes_downloaded\" => \"%1\",\n").arg(BytesToString2(stats.bytes_downloaded));
			out << QString("\"bytes_uploaded\" => \"%1\",\n").arg(BytesToString2(stats.bytes_uploaded));
			out << QString("\"bytes_left\" => %1,\n").arg(stats.bytes_left);
			out << QString("\"bytes_left_to_download\" => %1,\n").arg(stats.bytes_left_to_download);
			out << QString("\"total_bytes\" => \"%1\",\n").arg(BytesToString2(stats.total_bytes));
			out << QString("\"total_bytes_to_download\" => %1,\n").arg(stats.total_bytes_to_download);
			out << QString("\"download_rate\" => \"%1\",\n").arg(KBytesPerSecToString2(stats.download_rate / 1024.0));
			out << QString("\"upload_rate\" => \"%1\",\n").arg(KBytesPerSecToString2(stats.upload_rate / 1024.0));
			out << QString("\"num_peers\" => %1,\n").arg(stats.num_peers);
			out << QString("\"num_chunks_downloading\" => %1,\n").arg(stats.num_chunks_downloading);
			out << QString("\"total_chunks\" => %1,\n").arg(stats.total_chunks);
			out << QString("\"num_chunks_downloaded\" => %1,\n").arg(stats.num_chunks_downloaded);
			out << QString("\"num_chunks_excluded\" => %1,\n").arg(stats.num_chunks_excluded);
			out << QString("\"chunk_size\" => %1,\n").arg(stats.chunk_size);
			out << QString("\"seeders_total\" => %1,\n").arg(stats.seeders_total);
			out << QString("\"seeders_connected_to\" => %1,\n").arg(stats.seeders_connected_to);
			out << QString("\"leechers_total\" => %1,\n").arg(stats.leechers_total);
			out << QString("\"leechers_connected_to\" => %1,\n").arg(stats.leechers_connected_to);
			out << QString("\"status\" => %1,\n").arg(stats.status);
			out << QString("\"running\" => %1,\n").arg(stats.running);
			out << QString("\"trackerstatus\" => \"%1\",\n").arg(stats.trackerstatus.replace("\\", "\\\\").replace("\"", "\\\"").replace("$", "\\$"));
			out << QString("\"session_bytes_downloaded\" => %1,\n").arg(stats.session_bytes_downloaded);
			out << QString("\"session_bytes_uploaded\" => %1,\n").arg(stats.session_bytes_uploaded);
			out << QString("\"trk_bytes_downloaded\" => %1,\n").arg(stats.trk_bytes_downloaded);
			out << QString("\"trk_bytes_uploaded\" => %1,\n").arg(stats.trk_bytes_uploaded);
			out << QString("\"torrent_name\" => \"%1\",\n").arg(stats.torrent_name.replace("\\", "\\\\").replace("\"", "\\\"").replace("$", "\\$"));
			out << QString("\"output_path\" => \"%1\",\n").arg(stats.output_path.replace("\\", "\\\\").replace("\"", "\\\"").replace("$", "\\$"));
			out << QString("\"stopped_by_error\" => \"%1\",\n").arg(stats.stopped_by_error);
			out << QString("\"completed\" => \"%1\",\n").arg(stats.completed);
			out << QString("\"user_controlled\" => \"%1\",\n").arg(stats.user_controlled);
			out << QString("\"max_share_ratio\" => %1,\n").arg(stats.max_share_ratio);
			out << QString("\"priv_torrent\" => \"%1\",\n").arg(stats.priv_torrent);
			out << QString("\"num_files\" => \"%1\",\n").arg((*i)->getNumFiles());			
			out << QString("\"files\" => array(");
			out << flush;
			if (stats.multi_file_torrent)
			{
				//for loop to add each file+status to "files" array			
				for (Uint32 j = 0;j < (*i)->getNumFiles();j++)
				{
					if (j > 0)
						out << ",\n";
					
					TorrentFileInterface & file = (*i)->getTorrentFile(j);
					out << QString("\"%1\" => array(\n").arg(j);
					out << QString("\"name\" => \"%1\",\n").arg(file.getPath());
					out << QString("\"size\" => \"%1\",\n").arg(KIO::convertSize(file.getSize()));
					out << QString("\"perc_done\" => \"%1\",\n").arg(file.getDownloadPercentage());
					out << QString("\"status\" => \"%1\"\n").arg(file.getPriority());
					out << QString(")\n");
					out << flush;
				}
			}

			out << ")\n";
			out << ")\n";
		}
		
		out << ");\n}\n";
	}
	
	/*Generate php code
	* function globalStatus()
	* {
	*	return array( ... );
	* }
	*/
	void PhpCodeGenerator::globalInfo(QTextStream & out)
	{
		out << "function globalInfo()\n{\nreturn array(";
		CurrentStats stats=core->getStats();
	
		out << QString("\"download_speed\" => \"%1\",").arg(KBytesPerSecToString2(stats.download_speed / 1024.0));
		out << QString("\"upload_speed\" => \"%1\",").arg(KBytesPerSecToString2(stats.upload_speed / 1024.0));
		out << QString("\"bytes_downloaded\" => \"%1\",").arg(stats.bytes_downloaded);
		out << QString("\"bytes_uploaded\" => \"%1\",").arg(stats.bytes_uploaded);
		out << QString("\"max_download_speed\" => \"%1\",").arg(core->getMaxDownloadSpeed());
		out << QString("\"max_upload_speed\" => \"%1\",").arg(core->getMaxUploadSpeed());
		out << QString("\"max_downloads\" => \"%1\",").arg(Settings::maxDownloads());
		out << QString("\"max_seeds\"=> \"%1\",").arg(Settings::maxSeeds());
		out << QString("\"dht_support\" => \"%1\",").arg(Settings::dhtSupport());
		out << QString("\"use_encryption\" => \"%1\"").arg(Settings::useEncryption());
		out << ");\n}\n";
	}
	
	
	/************************
	*PhpActionExec		*
	************************/
	PhpActionExec::PhpActionExec(CoreInterface *c)
	{
		core=c;
	}
	
	bool PhpActionExec::exec(KURL & url,bool & shutdown)
	{
		bool ret = false;
		shutdown = false;
		int separator_loc;
		QString parse;
		QString torrent_num;
		QString file_num;
		KURL redirected_url;
		redirected_url.setPath(url.path());
		
		const QMap<QString, QString> & params = url.queryItems();
		QMap<QString, QString>::ConstIterator it;
	
		for ( it = params.begin(); it != params.end(); ++it ) 
		{
		//	Out(SYS_WEB| LOG_DEBUG) << "exec " << it.key().latin1() << endl;
			switch(it.key()[0])
			{
				case 'd':
					if(it.key()=="dht")
					{
						if(it.data()=="start")
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
							ht.start(kt::DataDir() + "dht_table",kt::DataDir() + "dht_key",Settings::dhtPort());
							ret = true;
						}
						else if (!Settings::dhtSupport() && ht.isRunning())
						{
							ht.stop();
							ret = true;
						}
						else if (Settings::dhtSupport() && ht.getPort() != Settings::dhtPort())
						{
							ht.stop();
							ht.start(kt::DataDir() + "dht_table",kt::DataDir() + "dht_key",Settings::dhtPort());
							ret = true;
						}
					}	
					break;
				case 'e':
					if(it.key()=="encription")
					{
						if(it.data()=="start")
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
						ret = true;
					}
					break;
				case 'f':
					//parse argument into torrent number and file number
					separator_loc=it.data().find('-');
					parse=it.data();
					torrent_num.append(parse.left(separator_loc));
					file_num.append(parse.right(parse.length()-(separator_loc+1)));

					if(it.key()=="file_lp")
					{
						QPtrList<TorrentInterface>::iterator i= core->getQueueManager()->begin();
						for(int k=0; i != core->getQueueManager()->end(); i++, k++)
						{
							if(torrent_num.toInt()==k)
							{
								TorrentFileInterface & file = (*i)->getTorrentFile(file_num.toInt());
								file.setPriority(LAST_PRIORITY);
								ret = true;
								break;
							}
						}
					}
					else if(it.key()=="file_np")
					{
						QPtrList<TorrentInterface>::iterator i= core->getQueueManager()->begin();
						for(int k=0; i != core->getQueueManager()->end(); i++, k++)
						{
							if(torrent_num.toInt()==k)
							{
								TorrentFileInterface & file = (*i)->getTorrentFile(file_num.toInt());
								file.setPriority(NORMAL_PRIORITY);
								ret = true;
								break;
							}
						}
					}
					else if(it.key()=="file_hp")
					{
						QPtrList<TorrentInterface>::iterator i= core->getQueueManager()->begin();
						for(int k=0; i != core->getQueueManager()->end(); i++, k++)
						{
							if(torrent_num.toInt()==k)
							{
								TorrentFileInterface & file = (*i)->getTorrentFile(file_num.toInt());
								file.setPriority(FIRST_PRIORITY);
								ret = true;
								break;
							}
						}
					}
					else if(it.key()=="file_stop")
					{
						QPtrList<TorrentInterface>::iterator i= core->getQueueManager()->begin();
						for(int k=0; i != core->getQueueManager()->end(); i++, k++)
						{
							if(torrent_num.toInt()==k)
							{
								TorrentFileInterface & file = (*i)->getTorrentFile(file_num.toInt());
								file.setPriority(ONLY_SEED_PRIORITY);
								ret = true;
								break;
							}
						}
					}
					break;
				case 'g':
					if(it.key()=="global_connection")
					{
						Settings::setMaxTotalConnections(it.data().toInt());
						PeerManager::setMaxTotalConnections(Settings::maxTotalConnections());
						ret = true;
					}
					break;
				case 'l':
					if(it.key()=="load_torrent" && it.data().length() > 0)
					{
						core->loadSilently(KURL::decode_string(it.data()));
						ret = true;
					}
					break;
				case 'm':
					if(it.key()=="maximum_downloads")
					{
						core->setMaxDownloads(it.data().toInt());
						Settings::setMaxDownloads(it.data().toInt());
						ret = true;
					}
					else if(it.key()=="maximum_seeds")
					{
						core->setMaxSeeds(it.data().toInt());
						Settings::setMaxSeeds(it.data().toInt());	
						ret = true;
					}
					else if(it.key()=="maximum_connection_per_torrent")
					{
						PeerManager::setMaxConnections(it.data().toInt());
						Settings::setMaxConnections(it.data().toInt());
						ret = true;
					}
					else if(it.key()=="maximum_upload_rate")
					{
						Settings::setMaxUploadRate(it.data().toInt());
						core->setMaxUploadSpeed(Settings::maxUploadRate());
						net::SocketMonitor::setUploadCap( Settings::maxUploadRate() * 1024);
						ret = true;
					}
					else if(it.key()=="maximum_download_rate")
					{
						Settings::setMaxDownloadRate(it.data().toInt());
						core->setMaxDownloadSpeed(Settings::maxDownloadRate());
						net::SocketMonitor::setDownloadCap(Settings::maxDownloadRate()*1024);
						ret = true;
					}
					else if(it.key()=="maximum_share_ratio")
					{
						Settings::setMaxRatio(it.data().toInt());
						ret = true;
					}
					break;
				case 'n':
					if(it.key()=="number_of_upload_slots")
					{
						Settings::setNumUploadSlots(it.data().toInt());
						Choker::setNumUploadSlots(Settings::numUploadSlots());
						ret = true;
					}
					break;
				case 'p':
					if(it.key()=="port")
					{
						Settings::setPort(it.data().toInt());
						core->changePort(Settings::port());
					}
					else if(it.key()=="port_udp_tracker")
					{
						Settings::setUdpTrackerPort(it.data().toInt());
						UDPTrackerSocket::setPort(Settings::udpTrackerPort());
						ret = true;
					}
					break;
				case 'q':
					if(it.key()=="quit" && !it.data().isEmpty())
					{
						shutdown = true;
						ret = true;
					}
					break;
				case 'r':
					if(it.key()=="remove")
					{
						QPtrList<TorrentInterface>::iterator i= core->getQueueManager()->begin();
						for(int k=0; i != core->getQueueManager()->end(); i++, k++)
						{
							if(it.data().toInt()==k)
							{
								core->remove((*i), false);
								ret = true;
								break;
							}
						}
					}
					break;
				case 's':
					if(it.key()=="stopall" && !it.data().isEmpty())
					{
						core->stopAll(3);
					}
					else if(it.key()=="startall" && !it.data().isEmpty())
					{
						core->startAll(3);
					}
					else if(it.key()=="stop")
					{
						QPtrList<TorrentInterface>::iterator i= core->getQueueManager()->begin();
						for(int k=0; i != core->getQueueManager()->end(); i++, k++)
						{
							if(it.data().toInt()==k)
							{
								(*i)->stop(true);
								ret = true;
								break;
							}
						}
					}
					else if(it.key()=="start")
					{
						QPtrList<TorrentInterface>::iterator i= core->getQueueManager()->begin();
						for(int k=0; i != core->getQueueManager()->end(); i++, k++)
						{
							if(it.data().toInt()==k)
							{
								(*i)->start();
								ret = true;
								break;
							}
						}
					}
					break;

				default:
					// add unknown query items to the redirected url
					// we don't add the keys above, because if the user presses refresh 
					// the same action will be taken again
					redirected_url.addQueryItem(it.key(),it.data());
					break;
			}
			Settings::writeConfig();	
		}
		
		if (ret)
			url = redirected_url; 
		
		return ret;
	}
	
	/************************
	*PhpInterface		*
	************************/
	PhpInterface::PhpInterface(CoreInterface *c):PhpCodeGenerator(c), PhpActionExec(c)
	{
	
	}

}
