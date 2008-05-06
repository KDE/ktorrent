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
#include "iwfiletreemodel.h"

#include <math.h>
#include <klocale.h>
#include <kglobal.h>
#include <util/functions.h>
#include <interfaces/functions.h>
#include <interfaces/torrentinterface.h>
#include <interfaces/torrentfileinterface.h>
#include "infowidgetpluginsettings.h"

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
			// xgettext: no-c-format
			case 4: return i18nc("Percent of File Downloaded", "% Complete");
			default: return QVariant();
		}
	}
	
	static QString PriorityString(const bt::TorrentFileInterface* file)
	{
		switch(file->getPriority())
		{
			case FIRST_PRIORITY: return i18nc("Download first", "First");
			case LAST_PRIORITY:	return i18nc("Download last", "Last");
			case ONLY_SEED_PRIORITY: 
			case EXCLUDED: 
			case PREVIEW_PRIORITY: 
				return QString();
			default:return i18nc("Download normally(not as first or last)", "Normal");
		}
	}
	
	QVariant IWFileTreeModel::data(const QModelIndex & index, int role) const
	{
		Node* n = 0;
		if (index.column() < 2 && role != Qt::ForegroundRole)
			return TorrentFileTreeModel::data(index,role);
		
		if (!index.isValid() || !(n = (Node*)index.internalPointer()))
			return QVariant();
		
		if (role == Qt::ForegroundRole && index.column() == 2 && tc->getStats().multi_file_torrent && n->file)
		{
			const bt::TorrentFileInterface* file = n->file;
			switch (file->getPriority())
			{
				case FIRST_PRIORITY:
					return InfoWidgetPluginSettings::firstColor();
				case LAST_PRIORITY:	
					return InfoWidgetPluginSettings::lastColor();
				case NORMAL_PRIORITY:
					return InfoWidgetPluginSettings::normalColor();
				case ONLY_SEED_PRIORITY: 
				case EXCLUDED: 
				case PREVIEW_PRIORITY: 
				default:
					return QVariant();
			}
		}
			
		if (role == Qt::DisplayRole)
			return displayData(n,index);
		else if (role == Qt::UserRole)
			return sortData(n,index);
		
		return QVariant();
	}

	QVariant IWFileTreeModel::displayData(Node* n,const QModelIndex & index) const
	{
		if (tc->getStats().multi_file_torrent && n->file)
		{
			const bt::TorrentFileInterface* file = n->file;
			switch (index.column())
			{
				case 2: return PriorityString(file);
				case 3: 
					if (file->isMultimedia())
					{
						if (file->isPreviewAvailable())
							return i18nc("preview available", "Available");
						else
							return i18nc("Preview pending", "Pending");
					}
					else
						return i18nc("No preview available", "No");
				case 4: 
				{
					float percent = file->getDownloadPercentage();
					return ki18n("%1 %").subs(percent, 0, 'g', 2).toString();
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
						if (tc->readyForPreview())
							return i18nc("Preview available", "Available");
						else
							return i18nc("Preview pending", "Pending");
					}
					else
						return i18nc("No preview available", "No");
				case 4: 
				{
					double percent = bt::Percentage(tc->getStats());
					return ki18n("%1 %").subs(percent, 0, 'g', 2).toString();
				}
				default: return QVariant();
			}
		}
		return QVariant();
	}
	
	QVariant IWFileTreeModel::sortData(Node* n,const QModelIndex & index) const
	{
		if (tc->getStats().multi_file_torrent && n->file)
		{
			const bt::TorrentFileInterface* file = n->file;
			switch (index.column())
			{
				case 2: return (int)file->getPriority();
				case 3: 
					if (file->isMultimedia())
					{
						if (file->isPreviewAvailable())
							return 3;
						else
							return 2;
					}
					else
						return 1;
				case 4: 
					return file->getDownloadPercentage();
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
						if (tc->readyForPreview())
							return 3;
						else
							return 2;
					}
					else
						return 1;
				case 4: 
					return bt::Percentage(tc->getStats());
			}
		}
		return QVariant();
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
			for (int i = 0;i < n->children.count();i++)
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
			for (int i = 0;i < n->children.count();i++)
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
			bool np = mmfile && tc->readyForPreview();
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
