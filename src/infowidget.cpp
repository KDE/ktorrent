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
#include <klistview.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kmimetype.h>
#include <qlabel.h>
#include <qtimer.h>
#include <libutil/functions.h>
#include <libutil/ptrmap.h>
#include <libtorrent/torrent.h>
#include <libtorrent/torrentcontrol.h>
#include "ktorrentmonitor.h"
#include "infowidget.h"
#include "peerview.h"
#include "chunkdownloadview.h"
#include "functions.h"
#include "chunkbar.h"

using namespace bt;

class IWFileTreeItem : public KListViewItem
{
	QString name;
	Uint32 size;
	bool is_dir;
	PtrMap<QString,IWFileTreeItem> children;
public:
	IWFileTreeItem(KListView* lv,const QString & name,bool dir,Uint32 size = 0)
	: KListViewItem(lv),name(name),size(size),is_dir(dir)
	{
		setText(0,name);
		if (!is_dir)
			setText(1,BytesToString(size));

		if (is_dir)
			setPixmap(0,KGlobal::iconLoader()->loadIcon("folder",KIcon::Small));
		else
			setPixmap(0,KMimeType::findByPath(name)->pixmap(KIcon::Small));
	}
			
	IWFileTreeItem(IWFileTreeItem* item,const QString & name,bool dir,Uint32 size = 0)
	: KListViewItem(item),name(name),size(size),is_dir(dir)
	{
		setText(0,name);
		if (!is_dir)
			setText(1,BytesToString(size));

		if (is_dir)
			setPixmap(0,KGlobal::iconLoader()->loadIcon("folder",KIcon::Small));
		else
			setPixmap(0,KMimeType::findByPath(name)->pixmap(KIcon::Small));
	}

	void insert(const QString & path,Uint32 size)
	{
		int p = path.find(bt::DirSeparator());
		if (p == -1)
		{
			children.insert(path,new IWFileTreeItem(this,path,false,size));
		}
		else
		{
			QString subdir = path.left(p);
			IWFileTreeItem* sd = children.find(subdir);
			if (!sd)
			{
				sd = new IWFileTreeItem(this,subdir,true);
				children.insert(subdir,sd);
			}
			
			sd->insert(path.mid(p+1),size);
		}
	}
};

InfoWidget::InfoWidget(QWidget* parent, const char* name, WFlags fl)
		: InfoWidgetBase(parent,name,fl)
{
	monitor = 0;
	curr_tc = 0;
	setEnabled(false);
	t = new QTimer(this);
	connect(t,SIGNAL(timeout()),this,SLOT(updateInfo()));
}

InfoWidget::~InfoWidget()
{
	delete monitor;
}

void InfoWidget::fillFileTree()
{
	m_file_view->clear();

	if (!curr_tc)
		return;

	const bt::Torrent & tor = curr_tc->getTorrent();
	if (tor.isMultiFile())
	{
		IWFileTreeItem* root = new IWFileTreeItem(m_file_view,tor.getNameSuggestion(),true);
		for (Uint32 i = 0;i < tor.getNumFiles();i++)
		{
			bt::TorrentFile file;
			tor.getFile(i,file);
			root->insert(file.getPath(),file.getSize());
		}
		root->setOpen(true);
		m_file_view->setRootIsDecorated(true);
	}
	else
	{
		m_file_view->setRootIsDecorated(false);
		new IWFileTreeItem(m_file_view,tor.getNameSuggestion(),false,tor.getFileLength());
	}
}

void InfoWidget::changeTC(bt::TorrentControl* tc)
{
	if (tc == curr_tc)
		return;

	if (tc)
		t->start(1000);
	else
		t->stop();
	
	curr_tc = tc;
	if (monitor)
	{
		delete monitor;
		monitor = 0;
		m_peer_view->removeAll();
		m_chunk_view->removeAll();
	}

	if (tc)
		monitor = new KTorrentMonitor(curr_tc,m_peer_view,m_chunk_view);

	fillFileTree();
	m_chunk_bar->setTC(tc);
	setEnabled(tc != 0);
	updateInfo();
}

void InfoWidget::updateInfo()
{
	if (!curr_tc)
		return;

	m_chunks_downloading->setText(QString::number(curr_tc->getNumChunksDownloading()));
	m_chunks_downloaded->setText(QString::number(curr_tc->getNumChunksDownloaded()));
	m_total_chunks->setText(QString::number(curr_tc->getTotalChunks()));
	m_chunk_bar->update();
}


#include "infowidget.moc"

