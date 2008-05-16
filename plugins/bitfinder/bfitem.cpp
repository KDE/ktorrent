/***************************************************************************
 *   Copyright (C) 2008 by Alan Jones                                      *
 *   skyphyr@gmail.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
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

#include <torrent/torrent.h>

#include "bfitem.h"

using namespace bt;

namespace kt
	{
	
	BFItem::BFItem(const QString& name, const QString& source, const QString& link, 
				const QString& description, const QByteArray& torrentData, QObject * parent) : 
					QObject(parent), name(name), source(source), link(link), 
					description(description), torrentData(torrentData)
		{
		
		}
	
	QString BFItem::getName() const
		{
		return name;
		}
		
	QString BFItem::getSource() const
		{
		return source;
		}
		
	QString BFItem::getLink() const
		{
		return link;
		}
		
	QString BFItem::getDescription() const
		{
		return description;
		}
		
	QByteArray BFItem::getTorrentData() const
		{
		return torrentData;
		}
	
	QStringList BFItem::getFilenames() const
		{
		QStringList filenames;
		
		Torrent curTorrent;
		
		try
			{
			curTorrent.load(torrentData, false);
			
			//we've loaded the torrentData and the torrent has no files
			if (!curTorrent.getNumFiles())
				return filenames;
				
			for (uint i=0; i<curTorrent.getNumFiles(); i++)
				{
				filenames << curTorrent.getFile(i).getPath();
				}
			}
		catch (...)
			{
			
			}
		
		return filenames;
		}
	
	bool BFItem::isValid() const
		{
		Torrent curTorrent;
		
		try
			{
			curTorrent.load(torrentData, false);
			
			//we've loaded the torrentData and the torrent has files
			if (curTorrent.getNumFiles())
				return true;
			}
		catch (...)
			{
			
			}
		
		return false;
		}
	
	}
