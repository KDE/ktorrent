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
 *   51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#include <qdir.h>
#include <qfileinfo.h>
#include <klocale.h>
#include <time.h>
#include <libutil/error.h>
#include "torrentcontrol.h"
#include "torrentcreator.h"
#include "bencoder.h"
#include <libutil/file.h>
#include <libutil/sha1hash.h>
#include <libutil/fileops.h>
#include <libutil/log.h>
#include <libutil/array.h>
#include <libutil/functions.h>
#include "globals.h"
#include "chunkmanager.h"

namespace bt
{

	TorrentCreator::TorrentCreator(const QString & tar,
								   const QStringList & track,
								   Uint32 cs,
								   const QString & name,
								   const QString & comments)
	: target(tar),trackers(track),chunk_size(cs),
	name(name),comments(comments),cur_chunk(0)
	{
		this->chunk_size *= 1024;
		QFileInfo fi(target);
		if (fi.isDir())
		{
			if (!this->target.endsWith(bt::DirSeparator()))
				this->target += bt::DirSeparator();
			
			Uint32 tot_size = 0;
			buildFileList("",tot_size);
			num_chunks = tot_size / chunk_size;
			if (tot_size % chunk_size > 0)
				num_chunks++;
			last_size = tot_size % chunk_size;
			Out() << "Tot Size : " << tot_size << endl;
		}
		else
		{
			num_chunks = fi.size() / chunk_size;
			if (fi.size() % chunk_size > 0)
					num_chunks++;
			last_size = fi.size() % chunk_size;
			Out() << "Tot Size : " << fi.size() << endl;
		}

		if (last_size == 0)
			last_size = chunk_size;

		Out() << "Num Chunks : " << num_chunks << endl;
		Out() << "Chunk Size : " << chunk_size << endl;
		Out() << "Last Size : " << last_size << endl;
	}


	TorrentCreator::~TorrentCreator()
	{}

	void TorrentCreator::buildFileList(const QString & dir,Uint32 & tot_size)
	{
		QDir d(target + dir);
		// first get all files (we ignore symlinks)
		QStringList dfiles = d.entryList(QDir::Files|QDir::NoSymLinks);
		for (QStringList::iterator i = dfiles.begin();i != dfiles.end();i++)
		{
			// add a Torrent::File to the list
			QFileInfo fi(target + dir + *i);
			Torrent::File f;
			f.path = dir + *i;
			f.size = fi.size();
			files.append(f);
			// update total size
			tot_size += fi.size();
		}

		// now for each subdir do a buildFileList 
		QStringList subdirs = d.entryList(QDir::Dirs|QDir::NoSymLinks);
		for (QStringList::iterator i = subdirs.begin();i != subdirs.end();i++)
		{
			QString sd = dir + *i;
			if (!sd.endsWith(bt::DirSeparator()))
				sd += bt::DirSeparator();
			buildFileList(sd,tot_size);
		}
	}


	void TorrentCreator::saveTorrent(const QString & url)
	{
		File fptr;
		if (!fptr.open(url,"wb"))
			throw Error(i18n("Cannot open file %1: %2").arg(url).arg(fptr.errorString()));

		BEncoder enc(&fptr);
		enc.beginDict(); // top dict
		enc.write("info");
		saveInfo(enc);
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
		enc.write("created by");enc.write("KTorrent 1.0");
		enc.write("creation date");enc.write(time(0));
		if (comments.length() > 0)
		{
			enc.write("comments");
			enc.write(comments);
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
			QValueList<Torrent::File>::iterator i = files.begin();
			while (i != files.end())
			{
				saveFile(enc,*i);
				i++;
			}
			enc.end();
		}
		else
		{
			enc.write("length"); enc.write(fi.size());
			
		}
		enc.write("name"); enc.write(name);
		enc.write("piece length"); enc.write(chunk_size);
		enc.write("pieces"); savePieces(enc);
		enc.end();
	}

	void TorrentCreator::saveFile(BEncoder & enc,const Torrent::File & file)
	{
		enc.beginDict();
		enc.write("length");enc.write(file.size);
		enc.write("path");
		enc.beginList();
		QStringList sl = QStringList::split(bt::DirSeparator(),file.path);
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
		for (Uint32 i = 0;i < num_chunks;i++)
		{
			memcpy(big_hash+(20*i),hashes[i].getData(),20);
		}
		enc.write(big_hash,num_chunks*20);
	}

	void TorrentCreator::calcChunkPos(Uint32 chunk,
									  int & f1,Uint32 & off,
									  Uint32 & size,int & f2)
	{
		// calculate in which file chunk lies and at what position

		// the number of bytes we have allready passed
		Uint32 bytes_passed = 0;
		// The byte offset the chunk begins
		Uint32 coff = chunk * chunk_size;
		Uint32 chunk_s = chunk != num_chunks - 1 ? chunk_size : last_size;
		for (Uint32 i = 0;i < files.count();i++)
		{
			Torrent::File f = files[i];

			if (coff < bytes_passed + f.size)
			{
				// bingo we found it
				if (coff + chunk_s <= bytes_passed + f.size)
				{
					// The chunk is in 1 file
					f1 = i;
					off = coff - bytes_passed;
					size = chunk_s;
					f2 = -1;
					return;
				}
				else
				{
					// The chunk is in 2 files

					// check if the next files exists
					if (i + 1 >= files.count())
						throw Error(i18n("Cannot find chunk"));
					
					f1 = i;
					off = coff - bytes_passed;
					size = chunk_s - ((coff + chunk_s) - (bytes_passed + f.size));
					f2 = i+1;
					return;
				}
			}
			else
			{
				// not found move to next file
				bytes_passed += f.size;
			}
		}

		// if we get here, we're in serious trouble
		throw Error(i18n("Cannot find chunk"));
	}

	bool TorrentCreator::calculateHash()
	{
		if (cur_chunk >= num_chunks)
			return true;

		Array<Uint8> buf(chunk_size);
		if (files.empty())
		{
			File fptr;
			if (!fptr.open(target,"rb"))
				throw Error(i18n("Cannot open file %1: %2")
						.arg(target).arg(fptr.errorString()));

			Uint32 s = cur_chunk != num_chunks - 1 ? chunk_size : last_size;
			fptr.seek(File::BEGIN,cur_chunk*chunk_size);
			
			fptr.read(buf,s);
			SHA1Hash h = SHA1Hash::generate(buf,s);
			hashes.append(h);
		}
		else
		{
			Uint32 s = cur_chunk != num_chunks - 1 ? chunk_size : last_size;
			// first find out where the chunk lies
			int f1,f2;
			Uint32 size,off;
			calcChunkPos(cur_chunk,f1,off,size,f2);

			// read from first file
			File fptr;
			if (!fptr.open(target + files[f1].path,"rb"))
				throw Error(i18n("Cannot open file %1: %2")
						.arg(target + files[f1].path)
						.arg(fptr.errorString()));

			fptr.seek(File::BEGIN,off);
			fptr.read(buf,size);

			// read from second file if necessary
			if (f2 > 0)
			{
				if (!fptr.open(target + files[f2].path,"rb"))
					throw Error(i18n("Cannot open file %1: %2")
							.arg(target + files[f2].path)
							.arg(fptr.errorString()));

				fptr.read(buf+size,s - size);
			}

			// generate hash
			SHA1Hash h = SHA1Hash::generate(buf,s);
			hashes.append(h);
		}
		cur_chunk++;
		return cur_chunk >= num_chunks;
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
		// link cache to real file/dir
		bt::SymLink(target,dd + "cache");
		// write full index file
		File fptr;
		if (!fptr.open(dd + "index","wb"))
			throw Error(i18n("Cannot create index file: %1").arg(fptr.errorString()));

		for (Uint32 i = 0;i < num_chunks;i++)
		{
			NewChunkHeader hdr;
			hdr.index = i;
			hdr.cache_off = i*chunk_size;
			fptr.write(&hdr,sizeof(NewChunkHeader));
		}
		fptr.close();

		// now create the torrentcontrol object
		TorrentControl* tc = new TorrentControl();
		try
		{
			tc->init(dd + "torrent",dd);
		}
		catch (...)
		{
			delete tc;
			throw;
		}
		
		return tc;
	}
}
