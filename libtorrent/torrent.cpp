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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include <qfile.h>
#include <qdatastream.h>
#include "log.h"
#include <time.h>
#include <stdlib.h>
#include "torrent.h"
#include "bdecoder.h"
#include "bnode.h"
#include "error.h"
#include "sha1hashgen.h"
#include "announcelist.h"

namespace bt
{

	Torrent::Torrent() : piece_length(0),file_length(0),anon_list(0)
	{}


	Torrent::~Torrent()
	{
		delete anon_list;
	}

	void Torrent::load(const QString & file)
	{
		QFile fptr(file);
		if (!fptr.open(IO_ReadOnly))
			throw Error(QString("Can't open file %1 : %2")
					.arg(file).arg(fptr.errorString()));
		
		
		QByteArray data(fptr.size());
	//	Out() << "File size = " << fptr.size() << endl;
		fptr.readBlock(data.data(),fptr.size());
	//	Out() << "Data size = " << data.size() << endl;
		
		BDecoder decoder(data);
		BNode* node = decoder.decode();
		BDictNode* dict = dynamic_cast<BDictNode*>(node);
		if (!node)
			throw Error("Corrupted torrent !");
		
	//	dict->printDebugInfo();
		try
		{
			loadTrackerURL(dict->getData("announce"));
			loadInfo(dict->getData("info"));
			loadAnnounceList(dict->getData("announce-list"));
			BNode* n = dict->getData("info");
			SHA1HashGen hg;
			Uint8* info = (Uint8*)data.data();
			info_hash = hg.generate(info + n->getOffset(),n->getLength());
		}
		catch (...)
		{
			delete dict;
			throw;
		}
	}
	
	void Torrent::loadInfo(BNode* info)
	{
		BDictNode* dict = dynamic_cast<BDictNode*>(info);
		if (!dict)
			throw Error("Corrupted torrent !");
		
		loadPieceLength(dict->getData("piece length"));
		BNode* n = dict->getData("length");
		if (n)
			loadFileLength(n);
		else
			loadFiles(dict->getData("files"));
		
		loadHash(dict->getData("pieces"));
		loadName(dict->getData("name"));
	}
	
	void Torrent::loadFiles(BNode* node)
	{
		Out() << "Multi file torrent" << endl;
		if (!node || node->getType() != BNode::LIST)
			throw Error("Corrupted torrent !");
		
		BListNode* fl = (BListNode*)node;
		for (Uint32 i = 0;i < fl->getNumChildren();i++)
		{
			BNode* n = fl->getChild(i);
			if (!n || n->getType() != BNode::DICT)
				throw Error("Corrupted torrent !");
			
			BDictNode* d = (BDictNode*)n;
			
			n = d->getData("length");
			if (!n || n->getType() != BNode::VALUE)
				throw Error("Corrupted torrent !");
			
			File file;
		
			BValueNode* v = (BValueNode*)n;
			file.size = v->data().toInt();
			file_length += file.size;
			
			n = d->getData("path");
			if (!n || n->getType() != BNode::LIST)
				throw Error("Corrupted torrent !");
			
			BListNode* ln = (BListNode*)n;
			for (Uint32 j = 0;j < ln->getNumChildren();j++)
			{
				n = ln->getChild(j);
				if (!n || n->getType() != BNode::VALUE)
					throw Error("Corrupted torrent !");
				
				v = (BValueNode*)n;
				file.path += v->data().toString();
				if (j + 1 < ln->getNumChildren())
					file.path += "/";
			}
			files.append(file);
		}
	}

	void Torrent::loadTrackerURL(BNode* node)
	{
		if (!node || node->getType() != BNode::VALUE)
			throw Error("Corrupted torrent !");
		
		BValueNode* v = (BValueNode*)node;
		if (v->data().getType() != Value::STRING)
			throw Error("Corrupted torrent !");
		
		tracker_url = v->data().toString();
	}
	
	void Torrent::loadPieceLength(BNode* node)
	{
		if (!node || node->getType() != BNode::VALUE)
			throw Error("Corrupted torrent !");
		
		BValueNode* v = (BValueNode*)node;
		if (v->data().getType() != Value::INT)
			throw Error("Corrupted torrent !");
		
		piece_length = v->data().toInt();
	}
	
	void Torrent::loadFileLength(BNode* node)
	{
		if (!node || node->getType() != BNode::VALUE)
			throw Error("Corrupted torrent !");
		
		BValueNode* v = (BValueNode*)node;
		if (v->data().getType() != Value::INT)
			throw Error("Corrupted torrent !");
		
		file_length = v->data().toInt();
	}
	
	void Torrent::loadHash(BNode* node)
	{
		if (!node || node->getType() != BNode::VALUE)
			throw Error("Corrupted torrent !");
		
		BValueNode* v = (BValueNode*)node;
		if (v->data().getType() != Value::STRING)
			throw Error("Corrupted torrent !");
		
		QByteArray hash_string = v->data().toByteArray();
		for (unsigned int i = 0;i < hash_string.size();i+=20)
		{
			Uint8 h[20];
			memcpy(h,hash_string.data()+i,20);
			SHA1Hash hash(h);
			hash_pieces.append(hash);
		}
	}
	
	void Torrent::loadName(BNode* node)
	{
		if (!node || node->getType() != BNode::VALUE)
			throw Error("Corrupted torrent !");
		
		BValueNode* v = (BValueNode*)node;
		if (v->data().getType() != Value::STRING)
			throw Error("Corrupted torrent !");
		
		name_suggestion = v->data().toString();
	}
	
	void Torrent::loadAnnounceList(BNode* node)
	{
		if (!node)
			return;
		
		if (anon_list)
		{
			delete anon_list;
			anon_list = 0;
		}
		
		anon_list = new AnnounceList();
		anon_list->load(node);
	}

	void Torrent::debugPrintInfo()
	{
		Out() << "Tracker URL : " << tracker_url << endl;
		Out() << "Name : " << name_suggestion << endl;
		Out() << "Piece Length : " << piece_length << endl;
		Out() << "File Length : " << file_length << endl;
		Out() << "Pieces : " << hash_pieces.size() << endl;
	}
	
	bool Torrent::verifyHash(const SHA1Hash & h,Uint32 index)
	{
		if (index >= hash_pieces.count())
			return false;
		
		const SHA1Hash & ph = hash_pieces[index];
		return ph == h;
	}
	
	const SHA1Hash & Torrent::getHash(Uint32 idx) const
	{
		return hash_pieces[idx];
	}
	
	void Torrent::getFile(Uint32 idx,Torrent::File & file)
	{
		if (idx >= files.size())
			return;
		
		file = files.at(idx);
	}
	
	KURL Torrent::getTrackerURL(bool last_was_succesfull) const
	{
		if (anon_list)
			return anon_list->getTrackerURL(last_was_succesfull);
		else
			return tracker_url;
	}
}
