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

#include <klocale.h>

namespace bt
{

	Torrent::Torrent() : piece_length(0),file_length(0),priv_torrent(false)
	{
		text_codec = QTextCodec::codecForName("utf-8");
		trackers = 0;
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
				throw Error(i18n("Corrupted torrent!"));

			// see if we can find an encoding node
			BValueNode* enc = dict->getValue("encoding");
			if (enc)
			{
				QTextCodec* tc = QTextCodec::codecForName(enc->data().toByteArray());
				if (tc)
				{
					Out(SYS_GEN|LOG_DEBUG) << "Encoding : " << QString(tc->name()) << endl;
					text_codec = tc;
				}
			}

			BValueNode* announce = dict->getValue("announce");
			BListNode* nodes = dict->getList("nodes");
			if (!announce && !nodes)
				throw Error(i18n("Torrent has no announce or nodes field"));
				
			if (announce)
				loadTrackerURL(announce);
			
			if (nodes) // DHT torrrents have a node key
				loadNodes(nodes);
			
			loadInfo(dict->getDict(QString("info")));
			loadAnnounceList(dict->getData("announce-list"));
			
			// see if the torrent contains webseeds
			BListNode* urls = dict->getList("url-list");
			BValueNode* url = dict->getValue("url-list");
			if (urls)
				loadWebSeeds(urls);
			else if (url)
				loadWebSeed(url);
			
			BNode* n = dict->getData("info");
			SHA1HashGen hg;
			Uint8* info = (Uint8*)data.data();
			info_hash = hg.generate(info + n->getOffset(),n->getLength());
			delete node;
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
			throw Error(i18n(" Unable to open torrent file %1 : %2")
					.arg(file).arg(fptr.errorString()));
		
		QByteArray data = fptr.readAll();
		load(data,verbose);
	}
	
	void Torrent::loadInfo(BDictNode* dict)
	{
		if (!dict)
			throw Error(i18n("Corrupted torrent!"));
		
		loadPieceLength(dict->getValue("piece length"));
		BValueNode* n = dict->getValue("length");
		if (n)
			loadFileLength(n);
		else
			loadFiles(dict->getList("files"));
		
		loadHash(dict->getValue("pieces"));
		loadName(dict->getValue("name"));
		n = dict->getValue("private");
		if (n && n->data().toInt() == 1)
			priv_torrent = true;
		
		// do a safety check to see if the number of hashes matches the file_length
		Uint32 num_chunks = (file_length / this->piece_length);
		if (file_length % piece_length > 0)
			num_chunks++;
		
		if (num_chunks != (Uint32)hash_pieces.count())
		{
			Out(SYS_GEN|LOG_DEBUG) << "File sizes and number of hashes do not match for " << name_suggestion << endl;
			throw Error(i18n("Corrupted torrent!"));
		}
	}
	
	void Torrent::loadFiles(BListNode* node)
	{
		Out() << "Multi file torrent" << endl;
		if (!node)
			throw Error(i18n("Corrupted torrent!"));
		Uint32 idx = 0;	
		BListNode* fl = node;
		for (Uint32 i = 0;i < fl->getNumChildren();i++)
		{
			BDictNode* d = fl->getDict(i);
			if (!d)
				throw Error(i18n("Corrupted torrent!"));
			
			BListNode* ln = d->getList("path");
			if (!ln)
				throw Error(i18n("Corrupted torrent!"));

			QString path;
			QList<QByteArray> unencoded_path;
			for (Uint32 j = 0;j < ln->getNumChildren();j++)
			{
				BValueNode* v = ln->getValue(j);
				if (!v || v->data().getType() != Value::STRING)
					throw Error(i18n("Corrupted torrent!"));
	
				unencoded_path.append(v->data().toByteArray());
				QString sd = v->data().toString(text_codec);
				path += sd;
				if (j + 1 < ln->getNumChildren())
					path += bt::DirSeparator();
			}

			// we do not want empty dirs
			if (path.endsWith(bt::DirSeparator()))
				continue;

			if (!checkPathForDirectoryTraversal(path))
				throw Error(i18n("Corrupted torrent!"));

			BValueNode* v = d->getValue("length");
			if (!v)
				throw Error(i18n("Corrupted torrent!"));

			if (v->data().getType() == Value::INT || v->data().getType() == Value::INT64)
			{
				Uint64 s = v->data().toInt64();
				TorrentFile file(idx,path,file_length,s,piece_length);
				file.setUnencodedPath(unencoded_path);

				// update file_length
				file_length += s;
				files.append(file);
			}
			else
			{
				throw Error(i18n("Corrupted torrent!"));
			}
			idx++;
		}
	}

	void Torrent::loadTrackerURL(BValueNode* node)
	{
		if (!node || node->data().getType() != Value::STRING)
			throw Error(i18n("Corrupted torrent!"));
		
		if (!trackers)
			trackers = new TrackerTier();
		
		trackers->urls.append(KUrl(node->data().toString(text_codec).trimmed()));
	}
	
	void Torrent::loadPieceLength(BValueNode* node)
	{
		if (!node)
			throw Error(i18n("Corrupted torrent!"));

		if (node->data().getType() == Value::INT)
			piece_length = node->data().toInt();
		else if (node->data().getType() == Value::INT64)
			piece_length = node->data().toInt64();
		else
			throw Error(i18n("Corrupted torrent!"));
	}
	
	void Torrent::loadFileLength(BValueNode* node)
	{
		if (!node)
			throw Error(i18n("Corrupted torrent!"));
				
		if (node->data().getType() == Value::INT)
			file_length = node->data().toInt();
		else if (node->data().getType() == Value::INT64)
			file_length = node->data().toInt64();
		else
			throw Error(i18n("Corrupted torrent!"));
	}
	
	void Torrent::loadHash(BValueNode* node)
	{
		if (!node || node->data().getType() != Value::STRING)
			throw Error(i18n("Corrupted torrent!"));
		
		
		QByteArray hash_string = node->data().toByteArray();
		for (int i = 0;i < hash_string.size();i+=20)
		{
			Uint8 h[20];
			memcpy(h,hash_string.data()+i,20);
			SHA1Hash hash(h);
			hash_pieces.append(hash);
		}
	}
	
	void Torrent::loadName(BValueNode* node)
	{
		if (!node || node->data().getType() != Value::STRING)
			throw Error(i18n("Corrupted torrent!"));
		
		unencoded_name = node->data().toByteArray();
		name_suggestion = text_codec->toUnicode(unencoded_name);
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
			BListNode* url = dynamic_cast<BListNode*>(ml->getChild(i));
			if (!url)
				throw Error(i18n("Parse Error"));
			
			for (Uint32 j = 0;j < url->getNumChildren();j++)
			{
				BValueNode* vn = dynamic_cast<BValueNode*>(url->getChild(j));
				if (!vn)
					throw Error(i18n("Parse Error"));

				KUrl url(vn->data().toString().trimmed());
				tier->urls.append(url);
				//Out() << "Added tracker " << url << endl;
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
				throw Error(i18n("Corrupted torrent!"));
			
			// first child is the IP, second the port
			BValueNode* ip = c->getValue(0);
			BValueNode* port = c->getValue(1);
			if (!ip || !port)
				throw Error(i18n("Corrupted torrent!"));
			
			if (ip->data().getType() != Value::STRING) 
				throw Error(i18n("Corrupted torrent!"));
			
			if (port->data().getType() != Value::INT)
				throw Error(i18n("Corrupted torrent!"));
			
			// add the DHT node
			DHTNode n;
			n.ip = ip->data().toString();
			n.port = port->data().toInt();
			nodes.append(n);
		}
	}
	
	void Torrent::loadWebSeeds(BListNode* node)
	{
		for (Uint32 i = 0;i < node->getNumChildren();i++)
		{
			loadWebSeed(node->getValue(i));
		}
	}
	
	void Torrent::loadWebSeed(BValueNode* node)
	{
		KUrl url = KUrl(node->data().toString(text_codec));
		if (url.isValid())
			web_seeds.append(url);
	}

	void Torrent::debugPrintInfo()
	{
		Out() << "Name : " << name_suggestion << endl;
		
//		for (KUrl::List::iterator i = tracker_urls.begin();i != tracker_urls.end();i++)
//			Out() << "Tracker URL : " << *i << endl;
		
		Out() << "Piece Length : " << piece_length << endl;
		if (this->isMultiFile())
		{
			Out() << "Files : " << endl;
			Out() << "===================================" << endl;
			for (Uint32 i = 0;i < getNumFiles();i++)
			{
				TorrentFile & tf = getFile(i);
				Out() << "Path : " << tf.getPath() << endl;
				Out() << "Size : " << tf.getSize() << endl;
				Out() << "First Chunk : " << tf.getFirstChunk() << endl;
				Out() << "Last Chunk : " << tf.getLastChunk() << endl;
				Out() << "First Chunk Off : " << tf.getFirstChunkOffset() << endl;
				Out() << "Last Chunk Size : " << tf.getLastChunkSize() << endl;
				Out() << "===================================" << endl;
			}
		}
		else
		{
			Out() << "File Length : " << file_length << endl;
		}
		Out() << "Pieces : " << hash_pieces.size() << endl;
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

		for (int i = 0;i < files.count();i++)
		{
			const TorrentFile & f = files[i];
			if (chunk >= f.getFirstChunk() && chunk <= f.getLastChunk() && f.getSize() != 0)
				file_list.append(f.getIndex());
		}
	}

	bool Torrent::isMultimedia() const
	{
		return IsMultimediaFile(this->getNameSuggestion());
	}
	
	void Torrent::updateFilePercentage(const BitSet & bs)
	{
		for (int i = 0;i < files.count();i++)
		{
			TorrentFile & f = files[i];
			f.updateNumDownloadedChunks(bs);
		}
	}
	
	void Torrent::updateFilePercentage(Uint32 chunk,const BitSet & bs)
	{
		QList<Uint32> cfiles;
		calcChunkPos(chunk,cfiles);
		
		QList<Uint32>::iterator i = cfiles.begin();
		while (i != cfiles.end())
		{
			TorrentFile & f = getFile(*i);
			f.updateNumDownloadedChunks(bs);
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
	}
}
