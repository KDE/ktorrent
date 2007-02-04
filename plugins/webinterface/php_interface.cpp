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

#include <net/socketmonitor.h>
#include <torrent/choker.h>
#include <torrent/udptrackersocket.h>
#include <kademlia/dhtbase.h>
#include <torrent/server.h>
#include <util/log.h>
#include <kapplication.h>

#include "php_interface.h"
namespace kt{
	QString DataDir();
}

using namespace bt;

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
	ret.append("function downloadStatus()\n{\nreturn ");
	ret.append("array(");

	QPtrList<TorrentInterface>::iterator i= core->getQueueManager()->begin();
	for(int k=0; i != core->getQueueManager()->end(); i++, k++)
        {
		stats=(*i)->getStats();
		ret.append(QString("%1 => array(").arg(k));
		
		ret.append(QString("\"imported_bytes\" => %1,").arg(stats.imported_bytes));
		ret.append(QString("\"bytes_downloaded\" => \"%1\",").arg(KIO::convertSize(stats.bytes_downloaded)));
		ret.append(QString("\"bytes_uploaded\" => \"%1\",").arg(KIO::convertSize(stats.bytes_uploaded)));
		ret.append(QString("\"bytes_left\" => %1,").arg(stats.bytes_left));
		ret.append(QString("\"bytes_left_to_download\" => %1,").arg(stats.bytes_left_to_download));
		ret.append(QString("\"total_bytes\" => \"%1\",").arg(KIO::convertSize(stats.total_bytes)));
		ret.append(QString("\"total_bytes_to_download\" => %1,").arg(stats.total_bytes_to_download));
		ret.append(QString("\"download_rate\" => \"%1/s\",").arg(KIO::convertSize(stats.download_rate)));
		ret.append(QString("\"upload_rate\" => \"%1/s\",").arg(KIO::convertSize(stats.upload_rate)));
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
		ret.append(QString("\"priv_torrent\" => \"%1\"").arg(stats.priv_torrent));


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

	ret.append(QString("\"download_speed\" => \"%1\",").arg(KIO::convertSize(stats.download_speed)));
	ret.append(QString("\"upload_speed\" => \"%1\",").arg(KIO::convertSize(stats.upload_speed)));
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

void PhpActionExec::exec(QMap<QString, QString> params)
{
QMap<QString, QString>::Iterator it;
		for ( it = params.begin(); it != params.end(); ++it ) {
				Out(SYS_WEB| LOG_DEBUG) << "exec " << it.key().latin1() << endl;
				switch(it.key()[0]){
				case 'd':
					if(it.key()=="dht"){
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
						}
						else if (!Settings::dhtSupport() && ht.isRunning())
						{
							ht.stop();
						}
						else if (Settings::dhtSupport() && ht.getPort() != Settings::dhtPort())
						{
							ht.stop();
							ht.start(kt::DataDir() + "dht_table",Settings::dhtPort());
						}
					}	
					else
						goto notsupported;
					break;
				case 'e':
					if(it.key()=="encription"){
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

						}
					else
						goto notsupported;
					break;
				case 'g':
					if(it.key()=="global_connection"){
						Settings::setMaxTotalConnections(it.data().toInt());
						PeerManager::setMaxTotalConnections(Settings::maxTotalConnections());
						}
					else
						goto notsupported;
					break;
				case 'l':
					if(it.key()=="load_torrent"){
						core->loadSilently(KURL::decode_string(it.data()));
						}
					else
						goto notsupported;
					break;
				case 'm':
					if(it.key()=="maximum_downloads"){
						core->setMaxDownloads(it.data().toInt());
						Settings::setMaxDownloads(it.data().toInt());
						}
					else if(it.key()=="maximum_seeds"){
						core->setMaxSeeds(it.data().toInt());
						Settings::setMaxSeeds(it.data().toInt());
						}
					else if(it.key()=="maximum_connection_per_torrent"){
						PeerManager::setMaxConnections(it.data().toInt());
						Settings::setMaxConnections(it.data().toInt());
						}
					else if(it.key()=="maximum_upload_rate"){
						Settings::setMaxUploadRate(it.data().toInt());
						core->setMaxUploadSpeed(Settings::maxUploadRate());
						net::SocketMonitor::setUploadCap( Settings::maxUploadRate() * 1024);
						}
					else if(it.key()=="maximum_download_rate"){
						Settings::setMaxDownloadRate(it.data().toInt());
						core->setMaxDownloadSpeed(Settings::maxDownloadRate());
						net::SocketMonitor::setDownloadCap(Settings::maxDownloadRate()*1024);
						}
					else if(it.key()=="maximum_share_ratio"){
						Settings::setMaxRatio(it.data().toInt());
						}
					else
						goto notsupported;
					break;
				case 'n':
					if(it.key()=="number_of_upload_slots"){
						Settings::setNumUploadSlots(it.data().toInt());
						Choker::setNumUploadSlots(Settings::numUploadSlots());
						}
					else
						goto notsupported;
					break;
				case 'p':
					if(it.key()=="port"){
						Settings::setPort(it.data().toInt());
						core->changePort(Settings::port());
						}
					else if(it.key()=="port_udp_tracker"){
						Settings::setUdpTrackerPort(it.data().toInt());
						UDPTrackerSocket::setPort(Settings::udpTrackerPort());
						}
					else
						goto notsupported;
					break;
				case 'q':
					if(it.key()=="quit"){
						if(!it.data().isEmpty())
							kapp->quit();
					}
					else
						goto notsupported;
					break;
				case 'r':
					if(it.key()=="remove"){
						QPtrList<TorrentInterface>::iterator i= core->getQueueManager()->begin();
						for(int k=0; i != core->getQueueManager()->end(); i++, k++)
						{
							if(it.data().toInt()==k){
								core->remove((*i), false);
								break;
								}
						}
					}
					else
						goto notsupported;
					break;
				case 's':
					if(it.key()=="stopall"){
						if(!it.data().isEmpty())
							core->stopAll(3);
					}
					else if(it.key()=="startall"){
						if(!it.data().isEmpty())
							core->startAll(3);
					}
					else if(it.key()=="stop"){
						QPtrList<TorrentInterface>::iterator i= core->getQueueManager()->begin();
						for(int k=0; i != core->getQueueManager()->end(); i++, k++)
						{
							if(it.data().toInt()==k){
								(*i)->stop(true);
								break;
								}
						}
					}
					else if(it.key()=="start"){
						QPtrList<TorrentInterface>::iterator i= core->getQueueManager()->begin();
						for(int k=0; i != core->getQueueManager()->end(); i++, k++)
						{
							if(it.data().toInt()==k){
								(*i)->start();
								break;
								}
						}
					}
					else
						goto notsupported;
					break;
				
				default:
notsupported:
					break;
				}
				Settings::writeConfig();
		                
		}
}

/************************
 *PhpInterface		*
 ************************/
PhpInterface::PhpInterface(CoreInterface *c):PhpCodeGenerator(c), PhpActionExec(c)
{

}

