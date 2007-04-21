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
#include <qdir.h>
#include <qfileinfo.h>
#include <klocale.h>
#include <time.h>
#include <util/error.h>
#include <ktversion.h>
#include "torrentcontrol.h"
#include "torrentcreator.h"
#include "bencoder.h"
#include <util/file.h>
#include <util/sha1hash.h>
#include <util/fileops.h>
#include <util/log.h>
#include <util/array.h>
#include <util/functions.h>
#include "globals.h"
#include "chunkmanager.h"
#include "statsfile.h"

namespace bt
{

	TorrentCreator::TorrentCreator(const QString & tar,
								   const QStringList & track,
								   Uint32 cs,
								   const QString & name,
								   const QString & comments,bool priv, bool decentralized)
	: target(tar),trackers(track),chunk_size(cs),
	name(name),comments(comments),cur_chunk(0),priv(priv),tot_size(0), decentralized(decentralized)
	{
		this->chunk_size *= 1024;
		QFileInfo fi(target);
		if (fi.isDir())
		{
			if (!this->target.endsWith(bt::DirSeparator()))
				this->target += bt::DirSeparator();
			
			tot_size = 0;
			buildFileList("");
			num_chunks = tot_size / chunk_size;
			if (tot_size % chunk_size > 0)
				num_chunks++;
			last_size = tot_size % chunk_size;
			Out() << "Tot Size : " << tot_size << endl;
		}
		else
		{
			tot_size = bt::FileSize(target);
			num_chunks = tot_size / chunk_size;
			if (tot_size % chunk_size > 0)
				num_chunks++;
			last_size = tot_size % chunk_size;
			Out() << "Tot Size : " << tot_size << endl;
		}

		if (last_size == 0)
			last_size = chunk_size;

		Out() << "Num Chunks : " << num_chunks << endl;
		Out() << "Chunk Size : " << chunk_size << endl;
		Out() << "Last Size : " << last_size << endl;
	}


	TorrentCreator::~TorrentCreator()
	{}

	void TorrentCreator::buildFileList(const QString & dir)
	{
		QDir d(target + dir);
		// first get all files (we ignore symlinks)
		QStringList dfiles = d.entryList(QDir::Files|QDir::NoSymLinks);
		Uint32 cnt = 0; // counter to keep track of file index
		for (QStringList::iterator i = dfiles.begin();i != dfiles.end();++i)
		{
			// add a TorrentFile to the list
			Uint64 fs = bt::FileSize(target + dir + *i);
			TorrentFile f(cnt,dir + *i,tot_size,fs,chunk_size);
			files.append(f);
			// update total size
			tot_size += fs;
			cnt++;
		}

		// now for each subdir do a buildFileList 
		QStringList subdirs = d.entryList(QDir::Dirs|QDir::NoSymLinks);
		for (QStringList::iterator i = subdirs.begin();i != subdirs.end();++i)
		{
			if (*i == "." || *i == "..")
				continue;
			
			QString sd = dir + *i;
			if (!sd.endsWith(bt::DirSeparator()))
				sd += bt::DirSeparator();
			buildFileList(sd);
		}
	}


	void TorrentCreator::saveTorrent(const QString & url)
	{
		File fptr;
		if (!fptr.open(url,"wb"))
			throw Error(i18n("Cannot open file %1: %2").arg(url).arg(fptr.errorString()));

		BEncoder enc(&fptr);
		enc.beginDict(); // top dict
		
		if(!decentralized)
		{
			enc.write("announce"); enc.write(trackers[0]);
			if (trackers.count() > 1)
			{
				enc.write("announce-list");
				enc.beginList();
				enc.beginList();
				for (Uint32 i = 0;i < trackers.count();i++)
					enc.write(trackers[i]);
				enc.end();
				enc.end();
			
			}
		}
		
		
		if (comments.length() > 0)
		{
			enc.write("comments");
			enc.write(comments);
		}
		enc.write("created by");enc.write(QString("KTorrent %1").arg(kt::VERSION_STRING));
		enc.write("creation date");enc.write((Uint64)time(0));
		enc.write("info");
		saveInfo(enc);
		// save the nodes list after the info hash, keys must be sorted !
		if (decentralized)
		{
			//DHT torrent
			enc.write("nodes");
			enc.beginList();
			
			for(int i=0; i < trackers.count(); ++i)
			{
				QString t = trackers[i];
				enc.beginList();
				enc.write(t.section(',',0,0));
				enc.write((Uint32)t.section(',',1,1).toInt());
				enc.end();
			}
			enc.end();
		}
		
		enc.end();
	}

	void TorrentCreator::saveInfo(BEncoder & enc)
	{
		enc.beginDict();
		
		QFileInfo fi(target);
		if (fi.isDir())
		{
			enc.write("files");
			enc.beginList();
			QValueList<TorrentFile>::iterator i = files.begin();
			while (i != files.end())
			{
				saveFile(enc,*i);
				i++;
			}
			enc.end();
		}
		else
		{
			enc.write("length"); enc.write(bt::FileSize(target));
		}
		enc.write("name"); enc.write(name);
		enc.write("piece length"); enc.write((Uint64)chunk_size);
		enc.write("pieces"); savePieces(enc);
		if (priv)
		{
			enc.write("private"); 
			enc.write((Uint64)1);
		}
		enc.end();
	}

	void TorrentCreator::saveFile(BEncoder & enc,const TorrentFile & file)
	{
		enc.beginDict();
		enc.write("length");enc.write(file.getSize());
		enc.write("path");
		enc.beginList();
		QStringList sl = QStringList::split(bt::DirSeparator(),file.getPath());
		for (QStringList::iterator i = sl.begin();i != sl.end();i++)
			enc.write(*i);
		enc.end();
		enc.end();
	}

	void TorrentCreator::savePieces(BEncoder & enc)
	{
		if (hashes.empty())
			while (!calculateHash())
				;

		Array<Uint8> big_hash(num_chunks*20);
		for (Uint32 i = 0;i < num_chunks;++i)
		{
			memcpy(big_hash+(20*i),hashes[i].getData(),20);
		}
		enc.write(big_hash,num_chunks*20);
	}

	bool TorrentCreator::calcHashSingle()
	{
		Array<Uint8> buf(chunk_size);
		File fptr;
		if (!fptr.open(target,"rb"))
			throw Error(i18n("Cannot open file %1: %2")
					.arg(target).arg(fptr.errorString()));

		Uint32 s = cur_chunk != num_chunks - 1 ? chunk_size : last_size;
		fptr.seek(File::BEGIN,(Int64)cur_chunk*chunk_size);
			
		fptr.read(buf,s);
		SHA1Hash h = SHA1Hash::generate(buf,s);
		hashes.append(h);
		cur_chunk++;
		return cur_chunk >= num_chunks;
	}
	
	bool TorrentCreator::calcHashMulti()
	{	
		Uint32 s = cur_chunk != num_chunks - 1 ? chunk_size : last_size;
		// first find the file(s) the chunk lies in
		Array<Uint8> buf(s);
		QValueList<TorrentFile> file_list;
		Uint32 i = 0;
		while (i < files.size())
		{
			const TorrentFile & tf = files[i];
			if (cur_chunk >= tf.getFirstChunk() && cur_chunk <= tf.getLastChunk())
			{
				file_list.append(tf);
			}
				
			i++;
		}

		Uint32 read = 0;
		for (i = 0;i < file_list.count();i++)
		{
			const TorrentFile & f = file_list[i];
			File fptr;
			if (!fptr.open(target + f.getPath(),"rb"))
			{
				throw Error(i18n("Cannot open file %1: %2")
						.arg(f.getPath()).arg(fptr.errorString()));
			}

			// first calculate offset into file
			// only the first file can have an offset
			// the following files will start at the beginning
			Uint64 off = 0;
			if (i == 0)
				off = f.fileOffset(cur_chunk,chunk_size);
			
			Uint32 to_read = 0;
			// then the amount of data we can read from this file
			if (file_list.count() == 1)
				to_read = s;
			else if (i == 0)
				to_read = f.getLastChunkSize();
			else if (i == file_list.count() - 1)
				to_read = s - read;
			else
				to_read = f.getSize();
						
			// read part of data
			fptr.seek(File::BEGIN,(Int64)off);
			fptr.read(buf + read,to_read);
			read += to_read;
		}

		// generate hash
		SHA1Hash h = SHA1Hash::generate(buf,s);
		hashes.append(h);

		cur_chunk++;
	//	Out() << "=============================================" << endl;
		return cur_chunk >= num_chunks;
	}
	
	bool TorrentCreator::calculateHash()
	{
		if (cur_chunk >= num_chunks)
			return true;
		if (files.empty())
			return calcHashSingle();
		else
			return calcHashMulti();
	}
	
	TorrentControl* TorrentCreator::makeTC(const QString & data_dir)
	{
		QString dd = data_dir;
		if (!dd.endsWith(bt::DirSeparator()))
			dd += bt::DirSeparator();

		// make data dir if necessary
		if (!bt::Exists(dd))
			bt::MakeDir(dd);

		// save the torrent
		saveTorrent(dd + "torrent");		
		// write full index file
		File fptr;
		if (!fptr.open(dd + "index","wb"))
			throw Error(i18n("Cannot create index file: %1").arg(fptr.errorString()));

		for (Uint32 i = 0;i < num_chunks;i++)
		{
			NewChunkHeader hdr;
			hdr.index = i;
			fptr.write(&hdr,sizeof(NewChunkHeader));
		}
		fptr.close();

		// now create the torrentcontrol object
		TorrentControl* tc = new TorrentControl();
		try
		{
			// get the parent dir of target
			QFileInfo fi = QFileInfo(target);
			
			QString odir;
			StatsFile st(dd + "stats");
			if (fi.fileName() == name)
			{
				st.write("OUTPUTDIR", fi.dirPath(true));
				odir = fi.dirPath(true);
			}
			else
			{
				st.write("CUSTOM_OUTPUT_NAME","1");
				st.write("OUTPUTDIR", target);
				odir = target;
			}
			st.write("UPLOADED", "0");
			st.write("RUNNING_TIME_DL","0");
			st.write("RUNNING_TIME_UL","0");
			st.write("PRIORITY", "0");
			st.write("AUTOSTART", "1");
			st.write("IMPORTED", QString::number(tot_size));
			st.writeSync();
			
			tc->init(0,dd + "torrent",dd,odir,QString::null);
			tc->createFiles();
		}
		catch (...)
		{
			delete tc;
			throw;
		}
		
		return tc;
	}
}
