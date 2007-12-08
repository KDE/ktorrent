/***************************************************************************
 *   Copyright (C) 2007 by Joris Guisson and Ivan Vasic                    *
 *   joris.guisson@gmail.com                                               *
 *   ivasic@gmail.com                                                      *
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
#include <math.h>
#include <klocale.h>
#include <kglobal.h>
#include <util/functions.h>
#include <interfaces/functions.h>
#include <interfaces/torrentinterface.h>
#include <interfaces/torrentfileinterface.h>
#include "iwfiletreemodel.h"

using namespace bt;

namespace kt
{

	IWFileTreeModel::IWFileTreeModel(bt::TorrentInterface* tc,QObject* parent)
			: TorrentFileTreeModel(tc,KEEP_FILES,parent)
	{
		mmfile = IsMultimediaFile(tc->getStats().output_path);
		preview = false;
		percentage = 0;
		for (Uint32 i = 0;i < tc->getNumFiles();i++)
		{
			bt::TorrentFileInterface & file = tc->getTorrentFile(i);
			connect(&file,SIGNAL(downloadPercentageChanged( float )),this,SLOT(onPercentageUpdated( float )));
			connect(&file,SIGNAL(previewAvailable( bool )),this,SLOT(onPreviewAvailable( bool )));
		}
	}


	IWFileTreeModel::~IWFileTreeModel()
	{
	}

	int IWFileTreeModel::columnCount(const QModelIndex & /*parent*/) const
	{
		return 5;
	}
	
	QVariant IWFileTreeModel::headerData(int section, Qt::Orientation orientation,int role) const
	{
		if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
			return QVariant();
		
		if (section < 2)
			return TorrentFileTreeModel::headerData(section,orientation,role);
		
		switch (section)
		{
			case 2: return i18n("Priority");
			case 3: return i18n("Preview");
			case 4: return i18n("% Complete");
			default: return QVariant();
		}
	}
	
	static QString PriorityString(const bt::TorrentFileInterface* file)
	{
		switch(file->getPriority())
		{
			case FIRST_PRIORITY: return i18n("First");
			case LAST_PRIORITY:	return i18n("Last");
			case ONLY_SEED_PRIORITY: 
			case EXCLUDED: 
			case PREVIEW_PRIORITY: 
				return QString::null;
			default:return i18n("Normal");
		}
	}
	
	QVariant IWFileTreeModel::data(const QModelIndex & index, int role) const
	{
		Node* n = 0;
		if (index.column() < 2)
			return TorrentFileTreeModel::data(index,role);
		
		if (!index.isValid() || !(n = (Node*)index.internalPointer()))
			return QVariant();
		
		if (role != Qt::DisplayRole)
			return QVariant();
		
		if (tc->getStats().multi_file_torrent && n->file)
		{
			const bt::TorrentFileInterface* file = n->file;
			switch (index.column())
			{
				case 2: return PriorityString(file);
				case 3: 
					if (file->isMultimedia())
					{
						if (tc->readyForPreview(file->getFirstChunk(), file->getFirstChunk()+1) )
							return i18n("Available");
						else
							return i18n("Pending");
					}
					else
						return i18n("No");
				case 4: 
				{
					float percent = file->getDownloadPercentage();
					KLocale* loc = KGlobal::locale();
					return i18n("%1 %",loc->formatNumber(percent,2));
				}
				default: return QVariant();
			}	
		}
		else if (!tc->getStats().multi_file_torrent)
		{
			switch (index.column())
			{
				case 2: return QVariant();
				case 3: 
					if (mmfile)
					{
						if (tc->readyForPreview(0,1))
							return i18n("Available");
						else
							return i18n("Pending");
					}
					else
						return i18n("No");
				case 4: 
				{
					double percent = bt::Percentage(tc->getStats());
					KLocale* loc = KGlobal::locale();
					return i18n("%1 %",loc->formatNumber(percent,2));
				}
				default: return QVariant();
			}
		}
		
		return QVariant();
	}

	bt::TorrentFileInterface* IWFileTreeModel::indexToFile(const QModelIndex & idx)
	{
		if (!idx.isValid())
			return 0;
		
		Node* n = (Node*)idx.internalPointer();
		if (!n)
			return 0;
		
		return n->file;
	}
	
	QString IWFileTreeModel::dirPath(const QModelIndex & idx)
	{
		if (!idx.isValid())
			return QString::null;
		
		Node* n = (Node*)idx.internalPointer();
		if (!n || n == root)
			return QString::null;
		
		QString ret = n->name;
		do 
		{
			n = n->parent;
			if (n && n->parent)
				ret = n->name + bt::DirSeparator() + ret;
		}while (n);
		
		return ret;
	}
	
	bool IWFileTreeModel::setData(const QModelIndex & index, const QVariant & value, int role)
	{
		if (role == Qt::CheckStateRole)
			return TorrentFileTreeModel::setData(index,value,role);
		
		if (!index.isValid() || role != Qt::UserRole)
			return false;
	
		Node* n = static_cast<Node*>(index.internalPointer());
		if (!n)
			return false;
		
		if (!n->file)
		{
			for (Uint32 i = 0;i < n->children.count();i++)
			{
				// recurse down the tree
				setData(index.child(i,0),value,role);
			}
		}
		else
		{
			bt::TorrentFileInterface* file = n->file;
			Priority prio = (bt::Priority)value.toInt();
			Priority old = file->getPriority();
			
			if (prio != old)
			{
				file->setPriority(prio);
				dataChanged(createIndex(index.row(),0),createIndex(index.row(),4));
				QModelIndex parent = index.parent();
				if (parent.isValid())
					dataChanged(parent,parent); // parent needs to be updated to 
			}
		}
		
		return true;
	}
	
	void IWFileTreeModel::changePriority(const QModelIndexList & indexes,bt::Priority newpriority)
	{
		foreach (QModelIndex idx,indexes)
		{
			Node* n = (Node*)idx.internalPointer();
			if (!n)
				continue;
			
			setData(idx,newpriority,Qt::UserRole);
		}
	}
	
	void IWFileTreeModel::missingFilesMarkedDND()
	{
		reset();
	}
	
	void IWFileTreeModel::onPercentageUpdated(float /*p*/)
	{
		bt::TorrentFileInterface* file = (bt::TorrentFileInterface*)sender();
		update(index(0,0,QModelIndex()),file,4);
	}
	
	void IWFileTreeModel::onPreviewAvailable(bool /*av*/)
	{
		bt::TorrentFileInterface* file = (bt::TorrentFileInterface*)sender();
		update(index(0,0,QModelIndex()),file,3);
	}
	
	void IWFileTreeModel::update(const QModelIndex & idx,bt::TorrentFileInterface* file,int col)
	{
		Node* n = (Node*)idx.internalPointer();
		if (n->file && n->file == file)
		{
			QModelIndex i = createIndex(idx.row(),col,n);
			emit dataChanged(i,i);
		}
		else
		{
			for (Uint32 i = 0;i < n->children.count();i++)
			{
				// recurse down the tree
				update(idx.child(i,0),file,col);
			}
		}
	}
	
	void IWFileTreeModel::update()
	{
		if (!tc->getStats().multi_file_torrent)
		{
			bool changed = false;
			bool np = mmfile && tc->readyForPreview(0,1);
			if (preview != np)
			{
				preview = np;
				changed = true;
			}
			
			double perc = bt::Percentage(tc->getStats());
			if (fabs(perc - percentage) > 0.01)
			{
				percentage = perc;
				changed = true;
			}
			
			if (changed)
				dataChanged(createIndex(0,0),createIndex(0,4));
		}
	}
}

#include "iwfiletreemodel.moc"