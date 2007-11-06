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
	QString PhpCodeGenerator::downloadStatus()
	{
		QString ret;
		TorrentStats stats;
		Priority file_priority;
		QString status;
		ret.append("function downloadStatus()\n{\nreturn ");
		ret.append("array(");

		QPtrList<TorrentInterface>::iterator i= core->getQueueManager()->begin();
		for(int k=0; i != core->getQueueManager()->end(); i++, k++)
			{
			stats=(*i)->getStats();
			ret.append(QString("%1 => array(").arg(k));
			
			ret.append(QString("\"imported_bytes\" => %1,").arg(stats.imported_bytes));
			ret.append(QString("\"bytes_downloaded\" => \"%1\",").arg(BytesToString2(stats.bytes_downloaded)));
			ret.append(QString("\"bytes_uploaded\" => \"%1\",").arg(BytesToString2(stats.bytes_uploaded)));
			ret.append(QString("\"bytes_left\" => %1,").arg(stats.bytes_left));
			ret.append(QString("\"bytes_left_to_download\" => %1,").arg(stats.bytes_left_to_download));
			ret.append(QString("\"total_bytes\" => \"%1\",").arg(BytesToString2(stats.total_bytes)));
			ret.append(QString("\"total_bytes_to_download\" => %1,").arg(stats.total_bytes_to_download));
			ret.append(QString("\"download_rate\" => \"%1\",").arg(KBytesPerSecToString2(stats.download_rate / 1024.0)));
			ret.append(QString("\"upload_rate\" => \"%1\",").arg(KBytesPerSecToString2(stats.upload_rate / 1024.0)));
			ret.append(QString("\"num_peers\" => %1,").arg(stats.num_peers));
			ret.append(QString("\"num_chunks_downloading\" => %1,").arg(stats.num_chunks_downloading));
			ret.append(QString("\"total_chunks\" => %1,").arg(stats.total_chunks));
			ret.append(QString("\"num_chunks_downloaded\" => %1,").arg(stats.num_chunks_downloaded));
			ret.append(QString("\"num_chunks_excluded\" => %1,").arg(stats.num_chunks_excluded));
			ret.append(QString("\"chunk_size\" => %1,").arg(stats.chunk_size));
			ret.append(QString("\"seeders_total\" => %1,").arg(stats.seeders_total));
			ret.append(QString("\"seeders_connected_to\" => %1,").arg(stats.seeders_connected_to));
			ret.append(QString("\"leechers_total\" => %1,").arg(stats.leechers_total));
			ret.append(QString("\"leechers_connected_to\" => %1,").arg(stats.leechers_connected_to));
			ret.append(QString("\"status\" => %1,").arg(stats.status));
			ret.append(QString("\"running\" => %1,").arg(stats.running));
			ret.append(QString("\"trackerstatus\" => \"%1\",").arg(stats.trackerstatus.replace("\\", "\\\\").replace("\"", "\\\"").replace("$", "\\$")));
			ret.append(QString("\"session_bytes_downloaded\" => %1,").arg(stats.session_bytes_downloaded));
			ret.append(QString("\"session_bytes_uploaded\" => %1,").arg(stats.session_bytes_uploaded));
			ret.append(QString("\"trk_bytes_downloaded\" => %1,").arg(stats.trk_bytes_downloaded));
			ret.append(QString("\"trk_bytes_uploaded\" => %1,").arg(stats.trk_bytes_uploaded));
			ret.append(QString("\"torrent_name\" => \"%1\",").arg(stats.torrent_name.replace("\\", "\\\\").replace("\"", "\\\"").replace("$", "\\$")));
			ret.append(QString("\"output_path\" => \"%1\",").arg(stats.output_path.replace("\\", "\\\\").replace("\"", "\\\"").replace("$", "\\$")));
			ret.append(QString("\"stopped_by_error\" => \"%1\",").arg(stats.stopped_by_error));
			ret.append(QString("\"completed\" => \"%1\",").arg(stats.completed));
			ret.append(QString("\"user_controlled\" => \"%1\",").arg(stats.user_controlled));
			ret.append(QString("\"max_share_ratio\" => %1,").arg(stats.max_share_ratio));
			ret.append(QString("\"priv_torrent\" => \"%1\",").arg(stats.priv_torrent));
			ret.append(QString("\"num_files\" => \"%1\",").arg((*i)->getNumFiles()));			
			ret.append(QString("\"files\" => array("));
			if (stats.multi_file_torrent)
			{
				//for loop to add each file+status to "files" array			
				for (Uint32 j = 0;j < (*i)->getNumFiles();j++)
				{
					TorrentFileInterface & file = (*i)->getTorrentFile(j);
					ret.append(QString("\"file_%1\" => \"%2\",").arg(j).arg(file.getPath()));
					ret.append(QString("\"size_%1\" => \"%2\",").arg(j).arg(KIO::convertSize(file.getSize())));
					ret.append(QString("\"perc_done_%1\" => \"%2\",").arg(j).arg(file.getDownloadPercentage()));

					file_priority=file.getPriority();
					if (file_priority==EXCLUDED)
						status="Do Not Download";
					else if (file_priority==LAST_PRIORITY)
						status="Download Last";
					else if (file_priority==NORMAL_PRIORITY)
						status="Download Normally";
					else if (file_priority==FIRST_PRIORITY)
						status="Download First";
					else if (file_priority == ONLY_SEED_PRIORITY)
						status="Only Seed";

					ret.append(QString("\"status_%1\" => \"%2\",").arg(j).arg(status));	
				}
				
			}

			ret.append("),");
			ret.append("),");
		}
		if(ret.endsWith(","))
			ret.truncate(ret.length()-1);
		ret.append(");\n}\n");
		return ret;
		
	}
	
	/*Generate php code
	* function globalStatus()
	* {
	*	return array( ... );
	* }
	*/
	QString PhpCodeGenerator::globalInfo()
	{
		QString ret;
		ret.append("function globalInfo()\n{\nreturn ");
		ret.append("array(");
		CurrentStats stats=core->getStats();
	
		ret.append(QString("\"download_speed\" => \"%1\",").arg(KBytesPerSecToString2(stats.download_speed / 1024.0)));
		ret.append(QString("\"upload_speed\" => \"%1\",").arg(KBytesPerSecToString2(stats.upload_speed / 1024.0)));
		ret.append(QString("\"bytes_downloaded\" => \"%1\",").arg(stats.bytes_downloaded));
		ret.append(QString("\"bytes_uploaded\" => \"%1\",").arg(stats.bytes_uploaded));
		ret.append(QString("\"max_download_speed\" => \"%1\",").arg(core->getMaxDownloadSpeed()));
		ret.append(QString("\"max_upload_speed\" => \"%1\",").arg(core->getMaxUploadSpeed()));
		ret.append(QString("\"max_downloads\" => \"%1\",").arg(Settings::maxDownloads()));
		ret.append(QString("\"max_seeds\"=> \"%1\",").arg(Settings::maxSeeds()));
		ret.append(QString("\"dht_support\" => \"%1\",").arg(Settings::dhtSupport()));
		ret.append(QString("\"use_encryption\" => \"%1\"").arg(Settings::useEncryption()));
		ret.append(");\n}\n");
	
		return ret;
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
							ht.start(kt::DataDir() + "dht_table",Settings::dhtPort());
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
							ht.start(kt::DataDir() + "dht_table",Settings::dhtPort());
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
								break;
							}
						}
					}
					else if(it.key()=="file_dnd")
					{
						QPtrList<TorrentInterface>::iterator i= core->getQueueManager()->begin();
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
