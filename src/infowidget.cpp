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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include <klistview.h>
#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kmimetype.h>
#include <kpopupmenu.h> 
#include <krun.h> 
#include <qlabel.h>
#include <qcheckbox.h>
#include <qpainter.h>
#include <libutil/functions.h>
#include <libtorrent/torrent.h>
#include <libtorrent/torrentcontrol.h>
#include "ktorrentmonitor.h"
#include "infowidget.h"
#include "peerview.h"
#include "chunkdownloadview.h"
#include "functions.h"
#include "downloadedchunkbar.h"
#include "availabilitychunkbar.h"
#include "iwfiletreeitem.h"

using namespace bt;



InfoWidget::InfoWidget(QWidget* parent, const char* name, WFlags fl)
		: InfoWidgetBase(parent,name,fl)
{
	multi_root = 0;
	monitor = 0;
	curr_tc = 0;
	setEnabled(false);

    KIconLoader* iload = KGlobal::iconLoader(); 
	context_menu = new KPopupMenu(this); 
	preview_id = context_menu->insertItem(iload->loadIconSet("frame_image",KIcon::Small), i18n("Preview")); 
	context_menu->setItemEnabled(1, false); 
 
    connect(m_file_view,SIGNAL(contextMenu(KListView*, QListViewItem*, const QPoint& )), 
            this,SLOT(showContextMenu(KListView*, QListViewItem*, const QPoint& ))); 
    connect(context_menu, SIGNAL ( activated ( int ) ), this, SLOT ( contextItem ( int ) ) );
}

InfoWidget::~InfoWidget()
{
	delete monitor;
}

void InfoWidget::fillFileTree()
{
	multi_root = 0;
	m_file_view->clear();

	if (!curr_tc)
		return;

	bt::Torrent & tor = const_cast<bt::Torrent &>(curr_tc->getTorrent());
	if (tor.isMultiFile())
	{
		IWFileTreeDirItem* root = new IWFileTreeDirItem(m_file_view,tor.getNameSuggestion());
		for (Uint32 i = 0;i < tor.getNumFiles();i++)
		{
			bt::TorrentFile & file = tor.getFile(i);
			root->insert(file.getPath(),file);
		}
		root->setOpen(true);
		m_file_view->setRootIsDecorated(true);
		multi_root = root;
	}
	else
	{
		m_file_view->setRootIsDecorated(false);
		KListViewItem* item = new KListViewItem(m_file_view,
						  tor.getNameSuggestion(),
						  BytesToString(tor.getFileLength()));

		QString name = tor.getNameSuggestion();
		item->setPixmap(0,KMimeType::findByPath(name)->pixmap(KIcon::Small));
	}
}

void InfoWidget::changeTC(bt::TorrentControl* tc)
{
	if (tc == curr_tc)
		return;
	
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
	m_av_chunk_bar->setTC(tc);
	setEnabled(tc != 0);
	update();
}

void InfoWidget::update()
{
	if (!curr_tc)
		return;

	m_chunks_downloading->setText(QString::number(curr_tc->getNumChunksDownloading()));
	m_chunks_downloaded->setText(QString::number(curr_tc->getNumChunksDownloaded()));
	m_total_chunks->setText(QString::number(curr_tc->getTotalChunks()));
	m_excluded_chunks->setText(QString::number(curr_tc->getNumChunksExcluded()));
	m_chunk_bar->update();
	m_av_chunk_bar->update();
	m_peer_view->update();
	m_chunk_view->update();
}

void InfoWidget::showContextMenu(KListView* ,QListViewItem* item,const QPoint & p) 
{ 

    bt::Torrent & tor = const_cast<bt::Torrent &>(curr_tc->getTorrent()); 

    if(tor.isMultiFile()) 
    {
        bt::TorrentFile & file = multi_root->findTorrentFile(item); 
        if ( !file.isNull() ) 
        { 
            if ( curr_tc->readyForPreview(file.getFirstChunk(), file.getFirstChunk()+1) ) 
            { 
                context_menu->setItemEnabled(preview_id, true); 
                this->preview_path = "cache" + bt::DirSeparator() + file.getPath(); 
            } 
            else context_menu->setItemEnabled(preview_id, false); 
        } 
    } 
    else 
    { 
        if ( curr_tc->readyForPreview() ) 
        { 
            context_menu->setItemEnabled(preview_id, true); 
            preview_path = "cache"; 
        } 
        else context_menu->setItemEnabled(preview_id, false); 
    } 
    context_menu->popup(p); 
} 
 
void InfoWidget::contextItem(int id) 
{ 
	if(id == this->preview_id) 
		KRun* exe = new KRun(this->curr_tc->getDataDir()+preview_path, 0, true, true);
} 



#include "infowidget.moc"

