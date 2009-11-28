/***************************************************************************
 *   Copyright (C) 2005 by                                                 *
 *   Joris Guisson <joris.guisson@gmail.com>                               *
 *   Ivan Vasic <ivasic@gmail.com>                                         *
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
#include "torrent.h"
#include <qfile.h>
#include <qdatastream.h>
#include <qstringlist.h>
#include <QTextCodec>
#include <util/log.h>
#include <util/functions.h>
#include <util/error.h>
#include <util/sha1hashgen.h>
#include <time.h>
#include <stdlib.h>
#include <bcodec/bdecoder.h>
#include <bcodec/bnode.h>
#include <interfaces/monitorinterface.h>

#include <klocale.h>

namespace bt
{
	static QString SanityzeName(const QString & name)
	{
#ifdef Q_WS_WIN
		QString ret = name;
		char invalid[] = {'<','>',':','"','/','\\','|','?','*'};
		for (int i = 0;i < 9;i++)
		{
			if (ret.contains(invalid[i]))
				ret = ret.replace(invalid[i],'_');
		}
		
		return ret;
#else
		if (name.endsWith("/"))
			return name.left(name.length() - 1);
		return name;
#endif
	}

	Torrent::Torrent() : chunk_size(0),total_size(0),priv_torrent(false),pos_cache_chunk(0),pos_cache_file(0),tmon(0)
	{
		text_codec = QTextCodec::codecForName("utf-8");
		trackers = 0;
		file_prio_listener = 0;
		loaded = false;
	}

	Torrent::Torrent(const bt::SHA1Hash & hash) : chunk_size(0),total_size(0),priv_torrent(false),pos_cache_chunk(0),pos_cache_file(0),tmon(0)
	{
		text_codec = QTextCodec::codecForName("utf-8");
		trackers = 0;
		file_prio_listener = 0;
		loaded = false;
		info_hash = hash;
	}

	Torrent::~Torrent()
	{
		delete trackers;
	}
	
	
	void Torrent::load(const QByteArray & data,bool verbose)
	{
		BNode* node = 0;
		 
		try
		{
			BDecoder decoder(data,verbose);
			node = decoder.decode();
			BDictNode* dict = dynamic_cast<BDictNode*>(node);
			if (!dict)
				throw Error(i18n("Corrupted torrent."));

			// see if we can find an encoding node
			if (dict->getValue("encoding"))
			{
				QByteArray enc = dict->getByteArray("encoding");
				QTextCodec* tc = QTextCodec::codecForName(enc);
				if (tc)
				{
					Out(SYS_GEN|LOG_DEBUG) << "Encoding : " << QString(tc->name()) << endl;
					text_codec = tc;
				}
			}
			
			BValueNode* c = dict->getValue("comment");
			if (c)
				comments = c->data().toString(text_codec);

			BValueNode* announce = dict->getValue("announce");
			BListNode* nodes = dict->getList("nodes");
			//if (!announce && !nodes)
			//	throw Error(i18n("Torrent has no announce or nodes field."));
				
			if (announce)
				loadTrackerURL(dict->getString("announce",text_codec));
			
			if (nodes) // DHT torrrents have a node key
				loadNodes(nodes);
			
			loadInfo(dict->getDict(QString("info")));
			loadAnnounceList(dict->getData("announce-list"));
			
			// see if the torrent contains webseeds
			BListNode* urls = dict->getList("url-list");
			if (urls)
			{
				loadWebSeeds(urls);
			}
			else if (dict->getValue("url-list"))
			{
				KUrl url(dict->getString("url-list",text_codec));
				if (url.isValid())
					web_seeds.append(url);
			}
			
			BNode* n = dict->getData("info");
			SHA1HashGen hg;
			// save info dict
			metadata = data.mid(n->getOffset(),n->getLength());
			info_hash = hg.generate((const Uint8*)metadata.data(),metadata.size());
			delete node;
			
			loaded = true;
		}
		catch (...)
		{
			delete node;
			throw;
		}
	}

	void Torrent::load(const QString & file,bool verbose)
	{
		QFile fptr(file);
		if (!fptr.open(QIODevice::ReadOnly))
			throw Error(i18n(" Unable to open torrent file %1 : %2", file, fptr.errorString()));
		
		QByteArray data = fptr.readAll();
		load(data,verbose);
	}
	
	void Torrent::loadInfo(BDictNode* dict)
	{
		if (!dict)
			throw Error(i18n("Corrupted torrent."));
		
		chunk_size = dict->getInt64("piece length");
		BListNode* files = dict->getList("files");
		if (files)
			loadFiles(files);
		else
			total_size = dict->getInt64("length");
		
		loadHash(dict);
		unencoded_name = dict->getByteArray("name");
		name_suggestion = text_codec->toUnicode(unencoded_name);
		name_suggestion = SanityzeName(name_suggestion);
		BValueNode* n = dict->getValue("private");
		if (n && n->data().toInt() == 1)
			priv_torrent = true;
		
		// do a safety check to see if the number of hashes matches the file_length
		Uint32 num_chunks = (total_size / chunk_size);
		last_chunk_size = total_size % chunk_size;
		if (last_chunk_size > 0)
			num_chunks++;
		else
			last_chunk_size = chunk_size;
		
		if (num_chunks != (Uint32)hash_pieces.count())
		{
			Out(SYS_GEN|LOG_DEBUG) << "File sizes and number of hashes do not match for " << name_suggestion << endl;
			throw Error(i18n("Corrupted torrent."));
		}
	}
	
	void Torrent::loadFiles(BListNode* node)
	{
		if (!node)
			throw Error(i18n("Corrupted torrent."));
		
		Uint32 idx = 0;
		BListNode* fl = node;
		for (Uint32 i = 0;i < fl->getNumChildren();i++)
		{
			BDictNode* d = fl->getDict(i);
			if (!d)
				throw Error(i18n("Corrupted torrent."));
			
			BListNode* ln = d->getList("path");
			if (!ln)
				throw Error(i18n("Corrupted torrent."));

			QString path;
			QList<QByteArray> unencoded_path;
			for (Uint32 j = 0;j < ln->getNumChildren();j++)
			{
				QByteArray v = ln->getByteArray(j);
				unencoded_path.append(v);
				QString sd = text_codec ? text_codec->toUnicode(v) : QString(v);
				if (sd.contains("\n"))
					sd = sd.remove("\n");
				path += sd;
				if (j + 1 < ln->getNumChildren())
					path += bt::DirSeparator();
			}

			// we do not want empty dirs
			if (path.endsWith(bt::DirSeparator()))
				continue;

			if (!checkPathForDirectoryTraversal(path))
				throw Error(i18n("Corrupted torrent."));

			Uint64 s = d->getInt64("length");
			TorrentFile file(this,idx,path,total_size,s,chunk_size);
			file.setUnencodedPath(unencoded_path);

			// update file_length
			total_size += s;
			files.append(file);
			idx++;
		}
	}

	void Torrent::loadTrackerURL(const QString & s)
	{
		if (!trackers)
			trackers = new TrackerTier();
	
		KUrl url(s);
		if (s.length() > 0 && url.isValid())
			trackers->urls.append(url);
	}
	
	void Torrent::loadHash(BDictNode* dict)
	{
		QByteArray hash_string = dict->getByteArray("pieces");
		for (int i = 0;i < hash_string.size();i+=20)
		{
			Uint8 h[20];
			memcpy(h,hash_string.data()+i,20);
			SHA1Hash hash(h);
			hash_pieces.append(hash);
		}
	}

	void Torrent::loadAnnounceList(BNode* node)
	{
		if (!node)
			return;
		
		BListNode* ml = dynamic_cast<BListNode*>(node);
		if (!ml)
			return;
		
		if (!trackers)
			trackers = new TrackerTier();
		
		TrackerTier* tier = trackers;
		//ml->printDebugInfo();
		for (Uint32 i = 0;i < ml->getNumChildren();i++)
		{
			BListNode* url_list = ml->getList(i);
			if (!url_list)
				throw Error(i18n("Parse Error"));
			
			for (Uint32 j = 0;j < url_list->getNumChildren();j++)
			{
				KUrl url(url_list->getString(j,0));
				tier->urls.append(url);
			}
			tier->next = new TrackerTier();
			tier = tier->next;
		}
	}
	
	void Torrent::loadNodes(BListNode* node)
	{
		for (Uint32 i = 0;i < node->getNumChildren();i++)
		{
			BListNode* c = node->getList(i);
			if (!c || c->getNumChildren() != 2)
				throw Error(i18n("Corrupted torrent."));
			
			// first child is the IP, second the port
			// add the DHT node
			DHTNode n;
			n.ip = c->getString(0,0);
			n.port = c->getInt(1);
			nodes.append(n);
		}
	}
	
	void Torrent::loadWebSeeds(BListNode* node)
	{
		for (Uint32 i = 0;i < node->getNumChildren();i++)
		{
			KUrl url = KUrl(node->getString(i,text_codec));
			if (url.isValid())
				web_seeds.append(url);
		}
	}

	void Torrent::debugPrintInfo()
	{
		Out(SYS_GEN|LOG_DEBUG) << "Name : " << name_suggestion << endl;
		
//		for (KUrl::List::iterator i = tracker_urls.begin();i != tracker_urls.end();i++)
//			Out(SYS_GEN|LOG_DEBUG) << "Tracker URL : " << *i << endl;
		
		Out(SYS_GEN|LOG_DEBUG) << "Piece Length : " << chunk_size << endl;
		if (this->isMultiFile())
		{
			Out(SYS_GEN|LOG_DEBUG) << "Files : " << endl;
			Out(SYS_GEN|LOG_DEBUG) << "===================================" << endl;
			for (Uint32 i = 0;i < getNumFiles();i++)
			{
				TorrentFile & tf = getFile(i);
				Out(SYS_GEN|LOG_DEBUG) << "Path : " << tf.getPath() << endl;
				Out(SYS_GEN|LOG_DEBUG) << "Size : " << tf.getSize() << endl;
				Out(SYS_GEN|LOG_DEBUG) << "First Chunk : " << tf.getFirstChunk() << endl;
				Out(SYS_GEN|LOG_DEBUG) << "Last Chunk : " << tf.getLastChunk() << endl;
				Out(SYS_GEN|LOG_DEBUG) << "First Chunk Off : " << tf.getFirstChunkOffset() << endl;
				Out(SYS_GEN|LOG_DEBUG) << "Last Chunk Size : " << tf.getLastChunkSize() << endl;
				Out(SYS_GEN|LOG_DEBUG) << "===================================" << endl;
			}
		}
		else
		{
			Out(SYS_GEN|LOG_DEBUG) << "File Length : " << total_size << endl;
		}
		Out(SYS_GEN|LOG_DEBUG) << "Pieces : " << hash_pieces.size() << endl;
	}
	
	bool Torrent::verifyHash(const SHA1Hash & h,Uint32 index)
	{
		if (index >= (Uint32)hash_pieces.count())
			return false;
		
		const SHA1Hash & ph = hash_pieces[index];
		return ph == h;
	}
	
	const SHA1Hash & Torrent::getHash(Uint32 idx) const
	{
		if (idx >= (Uint32)hash_pieces.count())
			throw Error(QString("Torrent::getHash %1 is out of bounds").arg(idx));
		
		return hash_pieces[idx];
	}
	
	TorrentFile & Torrent::getFile(Uint32 idx)
	{
		if (idx >= (Uint32)files.size())
			return TorrentFile::null;
		
		return files[idx];
	}

	const TorrentFile & Torrent::getFile(Uint32 idx) const
	{
		if (idx >= (Uint32)files.size())
			return TorrentFile::null;
		
		return files.at(idx);
	}

	unsigned int Torrent::getNumTrackerURLs() const
	{
		Uint32 count = 0;
		TrackerTier* tt = trackers;
		while (tt)
		{
			count += tt->urls.count();
			tt = tt->next;
		}
		return count;
	}

	void Torrent::calcChunkPos(Uint32 chunk,QList<Uint32> & file_list) const
	{
		file_list.clear();
		if (chunk >= (Uint32)hash_pieces.size() || files.empty())
			return;
		
		int start = (chunk >= this->pos_cache_chunk) ? this->pos_cache_file : 0;
		int end = (files.count() - 1);
		int mid = start + (end - start) / 2;
		while (start != mid && mid != end)
		{
			//printf("start = %i ; end = %i ; mid = %i\n",start,end,mid);
			const TorrentFile & f = files[mid];
			if (chunk >= f.getFirstChunk() && chunk <= f.getLastChunk())
			{
				int i = mid;
				while (i > 0)
				{
					i--;
					const TorrentFile & tf = files[i];
					if (!(chunk >= tf.getFirstChunk() && chunk <= tf.getLastChunk()))
					{
						i++;
						break;
					}
				}
				mid = i;
				break;
			}
			else
			{
				if (chunk > f.getLastChunk())
				{
					// chunk comes after file
					start = mid + 1;
					mid = start + (end - start) / 2;
				}
				else
				{
					// chunk comes before file
					end = mid - 1;
					mid = start + (end - start) / 2;
				}
			}
		}
		
		for (int i = mid;i < files.count();i++)
		{
			const TorrentFile & f = files[i];
			if (chunk >= f.getFirstChunk() && chunk <= f.getLastChunk() && f.getSize() != 0)
			{
				file_list.append(f.getIndex());
			}
			else if (chunk < f.getFirstChunk())
				break; 
		}
		
		pos_cache_chunk = chunk;
		pos_cache_file = file_list.at(0);
	}

	bool Torrent::isMultimedia() const
	{
		return IsMultimediaFile(this->getNameSuggestion());
	}
	
	void Torrent::updateFilePercentage(ChunkManager & cman)
	{
		for (int i = 0;i < files.count();i++)
		{
			TorrentFile & f = files[i];
			f.updateNumDownloadedChunks(cman);
		}
	}
	
	void Torrent::updateFilePercentage(Uint32 chunk,ChunkManager & cman)
	{
		QList<Uint32> cfiles;
		calcChunkPos(chunk,cfiles);
		
		QList<Uint32>::iterator i = cfiles.begin();
		while (i != cfiles.end())
		{
			TorrentFile & f = getFile(*i);
			f.updateNumDownloadedChunks(cman);
			i++;
		}
	}
	
	bool Torrent::checkPathForDirectoryTraversal(const QString & p)
	{
		QStringList sl = p.split(bt::DirSeparator());
		return !sl.contains("..");
	}
	
	void Torrent::changeTextCodec(QTextCodec* codec)
	{
		if (text_codec == codec)
			return;
		
		Out(SYS_GEN|LOG_DEBUG) << "Change Codec: " << QString(codec->name()) << endl;
		text_codec = codec;
		for (int i = 0;i < files.count();i++)
		{
			TorrentFile & f = files[i];
			f.changeTextCodec(codec);
		}
		name_suggestion = text_codec->toUnicode(unencoded_name);
		name_suggestion = SanityzeName(name_suggestion);
	}
	
	void Torrent::downloadPriorityChanged(TorrentFile* tf,Priority newpriority,Priority oldpriority)
	{
		if (file_prio_listener)
			file_prio_listener->downloadPriorityChanged(tf,newpriority,oldpriority);
	}
	
	void Torrent::filePercentageChanged(TorrentFile* tf,float perc)
	{
		if (tmon)
			tmon->filePercentageChanged(tf,perc);
	}
	
	void Torrent::filePreviewChanged(TorrentFile* tf,bool preview)
	{
		if (tmon)
			tmon->filePreviewChanged(tf,preview);
	}
}
