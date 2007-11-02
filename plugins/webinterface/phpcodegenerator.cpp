/***************************************************************************
 *   Copyright (C) 2006 by Diego R. Brogna and Joris Guisson               *
 *   dierbro@gmail.com                                               	   *
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
#include <klocale.h>
#include <kglobal.h>
#include <kio/global.h>
#include <settings.h>
#include <peer/peermanager.h>
#include <torrent/queuemanager.h>
#include <interfaces/coreinterface.h>
#include <interfaces/functions.h>
#include <interfaces/torrentinterface.h>
#include <interfaces/torrentfileinterface.h>
#include "phpcodegenerator.h"


namespace kt
{
	using bt::FIRST_PRIORITY;
	using bt::NORMAL_PRIORITY;
	using bt::LAST_PRIORITY;
	using bt::EXCLUDED;
	using bt::ONLY_SEED_PRIORITY;
	
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

	PhpCodeGenerator::PhpCodeGenerator(CoreInterface *c)
	{
		core=c;
	}
	
	PhpCodeGenerator::~PhpCodeGenerator()
	{}
	
	QString PhpCodeGenerator::downloadStatus()
	{
		QString ret;
		ret.append("function downloadStatus()\n{\nreturn ");
		ret.append("array(");
	
		QList<TorrentInterface*>::iterator i= core->getQueueManager()->begin();
		for(int k=0; i != core->getQueueManager()->end(); i++, k++)
		{
			const TorrentStats & stats = (*i)->getStats();
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
			QString tmp = stats.trackerstatus;
			ret.append(QString("\"trackerstatus\" => \"%1\",").arg(tmp.replace("\\", "\\\\").replace("\"", "\\\"").replace("$", "\\$")));
			ret.append(QString("\"session_bytes_downloaded\" => %1,").arg(stats.session_bytes_downloaded));
			ret.append(QString("\"session_bytes_uploaded\" => %1,").arg(stats.session_bytes_uploaded));
			ret.append(QString("\"trk_bytes_downloaded\" => %1,").arg(stats.trk_bytes_downloaded));
			ret.append(QString("\"trk_bytes_uploaded\" => %1,").arg(stats.trk_bytes_uploaded));
			
			tmp = stats.torrent_name;
			ret.append(QString("\"torrent_name\" => \"%1\",").arg(tmp.replace("\\", "\\\\").replace("\"", "\\\"").replace("$", "\\$")));
			tmp = stats.output_path;
			ret.append(QString("\"output_path\" => \"%1\",").arg(tmp.replace("\\", "\\\\").replace("\"", "\\\"").replace("$", "\\$")));
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
					Priority file_priority;
					QString status;
					
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
		ret.append(QString("\"max_download_speed\" => \"%1\",").arg(Settings::maxDownloadRate()));
		ret.append(QString("\"max_upload_speed\" => \"%1\",").arg(Settings::maxUploadRate()));
		ret.append(QString("\"max_downloads\" => \"%1\",").arg(Settings::maxDownloads()));
		ret.append(QString("\"max_seeds\"=> \"%1\",").arg(Settings::maxSeeds()));
		ret.append(QString("\"dht_support\" => \"%1\",").arg(Settings::dhtSupport()));
		ret.append(QString("\"use_encryption\" => \"%1\"").arg(Settings::useEncryption()));
		ret.append(");\n}\n");
	
		return ret;
	}
	
}
