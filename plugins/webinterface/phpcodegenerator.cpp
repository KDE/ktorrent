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
#include <util/functions.h>
#include <interfaces/torrentinterface.h>
#include <interfaces/torrentfileinterface.h>
#include "phpcodegenerator.h"

using namespace bt;

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
	
	void PhpCodeGenerator::downloadStatus(QTextStream & out)
	{
		QString ret;
		out << "function downloadStatus()\n{\nreturn ";
		out << "array(";
	
		QList<bt::TorrentInterface*>::iterator i= core->getQueueManager()->begin();
		for(int k=0; i != core->getQueueManager()->end(); i++, k++)
		{
			if (k > 0)
				out << ",\n";
			const TorrentStats & stats = (*i)->getStats();
			out << QString("%1 => array(").arg(k);
			
			out << QString("\"imported_bytes\" => %1,").arg(stats.imported_bytes);
			out << QString("\"bytes_downloaded\" => \"%1\",").arg(BytesToString2(stats.bytes_downloaded));
			out << QString("\"bytes_uploaded\" => \"%1\",").arg(BytesToString2(stats.bytes_uploaded));
			out << QString("\"bytes_left\" => %1,").arg(stats.bytes_left);
			out << QString("\"bytes_left_to_download\" => %1,").arg(stats.bytes_left_to_download);
			out << QString("\"total_bytes\" => \"%1\",").arg(BytesToString2(stats.total_bytes));
			out << QString("\"total_bytes_to_download\" => %1,").arg(stats.total_bytes_to_download);
			out << QString("\"download_rate\" => \"%1\",").arg(KBytesPerSecToString2(stats.download_rate / 1024.0));
			out << QString("\"upload_rate\" => \"%1\",").arg(KBytesPerSecToString2(stats.upload_rate / 1024.0));
			out << QString("\"num_peers\" => %1,").arg(stats.num_peers);
			out << QString("\"num_chunks_downloading\" => %1,").arg(stats.num_chunks_downloading);
			out << QString("\"total_chunks\" => %1,").arg(stats.total_chunks);
			out << QString("\"num_chunks_downloaded\" => %1,").arg(stats.num_chunks_downloaded);
			out << QString("\"num_chunks_excluded\" => %1,").arg(stats.num_chunks_excluded);
			out << QString("\"chunk_size\" => %1,").arg(stats.chunk_size);
			out << QString("\"seeders_total\" => %1,").arg(stats.seeders_total);
			out << QString("\"seeders_connected_to\" => %1,").arg(stats.seeders_connected_to);
			out << QString("\"leechers_total\" => %1,").arg(stats.leechers_total);
			out << QString("\"leechers_connected_to\" => %1,").arg(stats.leechers_connected_to);
			out << QString("\"status\" => %1,").arg(stats.status);
			out << QString("\"running\" => %1,").arg(stats.running);
			QString tmp = stats.tracker_status_string;
			out << QString("\"trackerstatus\" => \"%1\",").arg(tmp.replace("\\", "\\\\").replace("\"", "\\\"").replace("$", "\\$"));
			out << QString("\"session_bytes_downloaded\" => %1,").arg(stats.session_bytes_downloaded);
			out << QString("\"session_bytes_uploaded\" => %1,").arg(stats.session_bytes_uploaded);
			out << QString("\"trk_bytes_downloaded\" => %1,").arg(stats.trk_bytes_downloaded);
			out << QString("\"trk_bytes_uploaded\" => %1,").arg(stats.trk_bytes_uploaded);
			
			tmp = stats.torrent_name;
			out << QString("\"torrent_name\" => \"%1\",").arg(tmp.replace("\\", "\\\\").replace("\"", "\\\"").replace("$", "\\$"));
			tmp = (*i)->getDisplayName();
			out << QString("\"display_name\" => \"%1\",").arg(tmp.replace("\\", "\\\\").replace("\"", "\\\"").replace("$", "\\$"));
			tmp = stats.output_path;
			out << QString("\"output_path\" => \"%1\",").arg(tmp.replace("\\", "\\\\").replace("\"", "\\\"").replace("$", "\\$"));
			out << QString("\"stopped_by_error\" => \"%1\",").arg(stats.stopped_by_error);
			out << QString("\"completed\" => \"%1\",").arg(stats.completed);
			out << QString("\"user_controlled\" => \"%1\",").arg(stats.user_controlled);
			out << QString("\"max_share_ratio\" => %1,").arg(stats.max_share_ratio);
			out << QString("\"priv_torrent\" => \"%1\",").arg(stats.priv_torrent);
			out << QString("\"num_files\" => \"%1\",").arg((*i)->getNumFiles());			
			out << QString("\"files\" => array(");
			if (stats.multi_file_torrent)
			{
				//for loop to add each file+status to "files" array			
				for (Uint32 j = 0;j < (*i)->getNumFiles();j++)
				{
					if (j > 0)
						out << ",\n";
					TorrentFileInterface & file = (*i)->getTorrentFile(j);
					out << QString("\"%1\" => array(").arg(j);
					out << QString("\"name\" => \"%1\",").arg(file.getPath());
					out << QString("\"size\" => \"%1\",").arg(KIO::convertSize(file.getSize()));
					out << QString("\"perc_done\" => \"%1\",").arg(file.getDownloadPercentage());
					out << QString("\"status\" => \"%1\"").arg(file.getPriority());
					out << QString(")");	
				}
			}
			
			out << ")";
			out << ")";
		}
		
		out << ");\n}\n";		
	}
	
	void PhpCodeGenerator::globalInfo(QTextStream & out)
	{
		out << "function globalInfo()\n{\nreturn ";
		out << "array(";
		CurrentStats stats=core->getStats();
	
		out << QString("\"download_speed\" => \"%1\",").arg(KBytesPerSecToString2(stats.download_speed / 1024.0));
		out << QString("\"upload_speed\" => \"%1\",").arg(KBytesPerSecToString2(stats.upload_speed / 1024.0));
		out << QString("\"bytes_downloaded\" => \"%1\",").arg(stats.bytes_downloaded);
		out << QString("\"bytes_uploaded\" => \"%1\",").arg(stats.bytes_uploaded);
		out << QString("\"max_download_speed\" => \"%1\",").arg(Settings::maxDownloadRate());
		out << QString("\"max_upload_speed\" => \"%1\",").arg(Settings::maxUploadRate());
		out << QString("\"max_downloads\" => \"%1\",").arg(Settings::maxDownloads());
		out << QString("\"max_seeds\"=> \"%1\",").arg(Settings::maxSeeds());
#ifdef ENABLE_DHT_SUPPORT
		out << QString("\"dht_support\" => \"%1\",").arg(Settings::dhtSupport());
#else
		out << QString("\"dht_support\" => \"%1\",").arg(false);
#endif
		out << QString("\"use_encryption\" => \"%1\"").arg(Settings::useEncryption());
		out << ");\n}\n";
	}
	
}
