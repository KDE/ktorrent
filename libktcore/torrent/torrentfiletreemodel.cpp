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
#include <klocale.h>
#include <kicon.h>
#include <kmimetype.h>
#include <QTreeView>
#include <interfaces/torrentinterface.h>
#include <interfaces/torrentfileinterface.h>
#include <util/functions.h>
#include "torrentfiletreemodel.h"

using namespace bt;

namespace kt
{
	
	TorrentFileTreeModel::Node::Node(Node* parent,bt::TorrentFileInterface* file,const QString & name) 
		: parent(parent),file(file),name(name),size(0),expanded(true)
	{}
			
	TorrentFileTreeModel::Node::Node(Node* parent,const QString & name) 
		: parent(parent),file(0),name(name),size(0),expanded(true)
	{
	}
		
	TorrentFileTreeModel::Node::~Node()
	{
		qDeleteAll(children);
	}
		
	void TorrentFileTreeModel::Node::insert(const QString & path,bt::TorrentFileInterface* file)
	{
		int p = path.indexOf(bt::DirSeparator());
		if (p == -1)
		{
				// the file is part of this directory
			children.append(new Node(this,file,path));
		}
		else
		{
			QString subdir = path.left(p);
			foreach (Node* n,children)
			{
				if (n->name == subdir)
				{
					n->insert(path.mid(p+1),file);
					return;
				}
			}
						
			Node* n = new Node(this,subdir);
			children.append(n);
			n->insert(path.mid(p+1),file);
		}
	}
		
	int TorrentFileTreeModel::Node::row()
	{
		if (parent)
			return parent->children.indexOf(this);
		else
			return 0;
	}
		
	bt::Uint64 TorrentFileTreeModel::Node::fileSize(const bt::TorrentInterface* tc)
	{
		if (size > 0)
			return size;
			
		if (!file)
		{
			// directory
			foreach (Node* n,children)
				size += n->fileSize(tc);
		}
		else
		{
			size = file->getSize();
		}
		return size;
	}
		
	bt::Uint64 TorrentFileTreeModel::Node::bytesToDownload(const bt::TorrentInterface* tc)
	{
		bt::Uint64 s = 0;
			
		if (!file)
		{
				// directory
			foreach (Node* n,children)
				s += n->bytesToDownload(tc);
		}
		else
		{
			if (!file->doNotDownload())
				s = file->getSize();
		}
		return s;
	}
		
	Qt::CheckState TorrentFileTreeModel::Node::checkState(const bt::TorrentInterface* tc) const
	{
		if (!file)
		{
			bool found_checked = false;
			bool found_unchecked = false;
				// directory
			foreach (Node* n,children)
			{
				Qt::CheckState s = n->checkState(tc);
				if (s == Qt::PartiallyChecked)
					return s;
				else if (s == Qt::Checked)
					found_checked = true;
				else
					found_unchecked = true;
					
				if (found_checked && found_unchecked)
					return Qt::PartiallyChecked;
			}
				
			return found_checked ? Qt::Checked : Qt::Unchecked;
		}
		else
		{
			return file->doNotDownload() || file->getPriority() == ONLY_SEED_PRIORITY ? Qt::Unchecked : Qt::Checked;
		}
	}
	
	void TorrentFileTreeModel::Node::saveExpandedState(const QModelIndex & index,QTreeView* tv)
	{
		if (file)
			return;
		
		expanded = tv->isExpanded(index);
		
		int idx = 0;
		foreach (Node* n,children)
		{
			if (!n->file)
				n->saveExpandedState(index.child(idx,0),tv);
			idx++;
		}
	}
	
	void TorrentFileTreeModel::Node::loadExpandedState(const QModelIndex & index,QTreeView* tv)
	{
		if (file)
			return;
		
		tv->setExpanded(index,expanded);
		
		int idx = 0;
		foreach (Node* n,children)
		{
			if (!n->file)
				n->loadExpandedState(index.child(idx,0),tv);
			idx++;
		}
	}

	TorrentFileTreeModel::TorrentFileTreeModel(bt::TorrentInterface* tc,DeselectMode mode,QObject* parent) 
	: QAbstractItemModel(parent),tc(tc),root(0),mode(mode),emit_check_state_change(true)
	{
		if (tc->getStats().multi_file_torrent)
			constructTree();
		else
			root = new Node(0,tc->getStats().torrent_name);
	}


	TorrentFileTreeModel::~TorrentFileTreeModel()
	{}
	
	void TorrentFileTreeModel::constructTree()
	{
		if (!root)
			root = new Node(0,tc->getStats().torrent_name);
		
		for (Uint32 i = 0;i < tc->getNumFiles();i++)
		{
			bt::TorrentFileInterface & tf = tc->getTorrentFile(i);
			root->insert(tf.getPath(),&tf);
		}
	}

	int TorrentFileTreeModel::rowCount(const QModelIndex & parent) const
	{	
		if (!parent.isValid())
		{
			return 1;
		}
		else
		{
			Node* n = (Node*)parent.internalPointer();
			return n->children.count();
		}
	}
	
	int TorrentFileTreeModel::columnCount(const QModelIndex & parent) const
	{
		if (!parent.isValid())
			return 2;
		else
			return 2;
	}
	
	QVariant TorrentFileTreeModel::headerData(int section, Qt::Orientation orientation,int role) const
	{
		if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
			return QVariant();
		 
		switch (section)
		{
			case 0: return i18n("File");
			case 1: return i18n("Size");
			default:
				return QVariant();
		}
	}
	
	QVariant TorrentFileTreeModel::data(const QModelIndex & index, int role) const
	{
		if (!index.isValid())
			return QVariant();
			
		Node* n = (Node*)index.internalPointer();
		if (!n)
			return QVariant();
			
		if (role == Qt::DisplayRole)
		{
			switch (index.column())
			{
				case 0: return n->name;
				case 1: 
					if (tc->getStats().multi_file_torrent)
						return BytesToString(n->fileSize(tc));
					else
						return BytesToString(tc->getStats().total_bytes);
				default: return QVariant();
			}
		}
		else if (role == Qt::DecorationRole && index.column() == 0)
		{
			// if this is an empty folder then we are in the single file case
			if (!n->file)
				return n->children.count() > 0 ? 
						KIcon("folder") : KIcon(KMimeType::findByPath(tc->getStats().torrent_name)->iconName());
			else
				return KIcon(KMimeType::findByPath(n->file->getPath())->iconName());
		}
		else if (role == Qt::CheckStateRole && index.column() == 0)
		{
			if (tc->getStats().multi_file_torrent)
				return n->checkState(tc);
		}
		
		return QVariant();
	}
	
	QModelIndex TorrentFileTreeModel::parent(const QModelIndex & index) const
	{
		if (!index.isValid())
			return QModelIndex();

		Node* child = static_cast<Node*>(index.internalPointer());
		if (!child)
			return QModelIndex();
		
		Node* parent = child->parent;
		if (!parent)
			return QModelIndex();
		else
			return createIndex(parent->row(), 0, parent);
	}
	
	QModelIndex TorrentFileTreeModel::index(int row,int column,const QModelIndex & parent) const
	{
		if (!hasIndex(row, column, parent))
			return QModelIndex();

		Node* p = 0;

		if (!parent.isValid())
			return createIndex(row,column,root);
		else
		{
			p = static_cast<Node*>(parent.internalPointer());

			if (row >= 0 && row < p->children.count())
				return createIndex(row,column,p->children.at(row));
			else
				return QModelIndex();
		}
	}
	
	Qt::ItemFlags TorrentFileTreeModel::flags(const QModelIndex & index) const
	{
		if (!index.isValid())
			return 0;
		else if (tc->getStats().multi_file_torrent)
			return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
		else
			return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
	}
	
	bool TorrentFileTreeModel::setData(const QModelIndex & index, const QVariant & value, int role) 
	{
		if (!index.isValid() || role != Qt::CheckStateRole)
			return false;
		
		Node* n = static_cast<Node*>(index.internalPointer());
		if (!n)
			return false;
		
		Qt::CheckState newState = static_cast<Qt::CheckState>(value.toInt());
		if (!n->file)
		{
			bool reenable = false;
			if (emit_check_state_change)
			{
				reenable = true;
				emit_check_state_change = false;
			}
			
			for (Uint32 i = 0;i < n->children.count();i++)
			{
				// recurse down the tree
				setData(index.child(i,0),value,role);
			}
			
			if (reenable)
				emit_check_state_change = true;
		}
		else
		{
			bt::TorrentFileInterface* file = n->file;
			if (newState == Qt::Checked)
			{
				if (file->getPriority() == ONLY_SEED_PRIORITY)
					file->setPriority(NORMAL_PRIORITY);
				else
					file->setDoNotDownload(false);
			}
			else
			{
				if (mode == KEEP_FILES)
					file->setPriority(ONLY_SEED_PRIORITY);
				else
					file->setDoNotDownload(true);
			}
			dataChanged(createIndex(index.row(),0),createIndex(index.row(),columnCount(index) - 1));
			
			QModelIndex parent = index.parent();
			if (parent.isValid())
				dataChanged(parent,parent); // parent needs to be updated to 
		}
		
		if (emit_check_state_change)
			checkStateChanged();
		return true;
	}
	
	void TorrentFileTreeModel::checkAll()
	{
		if (tc->getStats().multi_file_torrent)
			setData(index(0,0,QModelIndex()),Qt::Checked,Qt::CheckStateRole);
	}
		
	void TorrentFileTreeModel::uncheckAll()
	{
		if (tc->getStats().multi_file_torrent)
			setData(index(0,0,QModelIndex()),Qt::Unchecked,Qt::CheckStateRole);
	}
	
	void TorrentFileTreeModel::invertCheck()
	{
		if (!tc->getStats().multi_file_torrent)
			return;
		
		invertCheck(index(0,0,QModelIndex()));
	}
	
	void TorrentFileTreeModel::invertCheck(const QModelIndex & idx)
	{
		Node* n = static_cast<Node*>(idx.internalPointer());
		if (!n)
			return;
		
		if (!n->file)
		{
			for (Uint32 i = 0;i < n->children.count();i++)
			{
				// recurse down the tree
				invertCheck(idx.child(i,0));
			}
		}
		else
		{
			if (n->file->doNotDownload())
				setData(idx,Qt::Checked,Qt::CheckStateRole);
			else
				setData(idx,Qt::Unchecked,Qt::CheckStateRole);
		}
	}
	
	bt::Uint64 TorrentFileTreeModel::bytesToDownload()
	{
		if (tc->getStats().multi_file_torrent)
			return root->bytesToDownload(tc);
		else
			return tc->getStats().total_bytes;
	}
	
	void TorrentFileTreeModel::saveExpandedState(QTreeView* tv)
	{
		if (tc->getStats().multi_file_torrent)
			root->saveExpandedState(index(0,0,QModelIndex()),tv);
	}
		
		
	void TorrentFileTreeModel::loadExpandedState(QTreeView* tv)
	{
		if (tc->getStats().multi_file_torrent)
			root->loadExpandedState(index(0,0,QModelIndex()),tv);
	}
}

#include "torrentfiletreemodel.moc"
