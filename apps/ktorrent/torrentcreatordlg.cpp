//
// C++ Implementation: $MODULE$
//
// Description: 
//
//
// Author: Joris Guisson <joris.guisson@gmail.com>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include <qcheckbox.h>
#include <qstringlist.h>
#include <qmap.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kcombobox.h>
#include <klineedit.h>
#include <kurlrequester.h>
#include <keditlistbox.h>
#include <kpushbutton.h>
#include <kfiledialog.h>
#include <kprogress.h>
#include <klistview.h>
#include <knuminput.h>
#include "torrentcreatordlg.h"
#include "ktorrentcore.h"
#include <torrent/globals.h>
#include <kademlia/dhtbase.h>
		
TorrentCreatorDlg::TorrentCreatorDlg(KTorrentCore* core,QWidget *parent, const char *name)
	:TorrentCreatorDlgBase(parent, name),core(core)
{
	KURLRequester* r = m_file_or_dir;
	r->fileDialog()->setMode(
			KFile::ExistingOnly|KFile::Directory|KFile::File|KFile::LocalOnly);

	KComboBox* cb = m_chunk_size;
	cb->setCurrentItem(3);
	
	connect(m_create_btn,SIGNAL(clicked()),this,SLOT(onCreate()));
	connect(m_cancel_btn,SIGNAL(clicked()),this,SLOT(reject()));
	
	m_nodes->setHidden(true);
	
	QMap<QString, int> n = bt::Globals::instance().getDHT().getClosestGoodNodes(10);
	
	for(QMap<QString, int>::iterator it = n.begin(); it!=n.end(); ++it)
		new QListViewItem(m_nodeList, it.key(), QString("%1").arg(it.data()));
}

TorrentCreatorDlg::~TorrentCreatorDlg()
{
}

void TorrentCreatorDlg::onCreate()
{
	KURLRequester* r = m_file_or_dir;
	KComboBox* cb = m_chunk_size;
	KEditListBox* eb = m_trackers;
	
	if (r->url().length() == 0)
	{
		errorMsg(i18n("You must select a file or a folder."));
		return;
	}

	if (eb->items().count() == 0 && !m_decentralized->isChecked())
	{
		//errorMsg(i18n("You must add at least one tracker."));
		QString msg = i18n("You have not added a tracker, "
				"are you sure you want to create this torrent ?");
		if (KMessageBox::warningYesNo(this,msg) == KMessageBox::No)
			return;
	}
	
	if (m_nodeList->childCount() == 0 && m_decentralized->isChecked())
	{
		errorMsg(i18n("You must add at least one node."));
		return;
	}

	QString url = r->url();
	int chunk_size = cb->currentText().toInt();
	QString name = KURL::fromPathOrURL(r->url()).fileName();
	
	QStringList trackers; 
	
	if(m_decentralized->isChecked())
	{
		for(int i=0; i<m_nodeList->childCount(); ++i)
			trackers.append(m_nodeList->itemAtIndex(i)->text(0) + "," +  m_nodeList->itemAtIndex(i)->text(1));
	}
	else
		trackers = eb->items();

	QString s = KFileDialog::getSaveFileName(
			QString::null,"*.torrent|" + i18n("Torrent Files (*.torrent)"),
			0,i18n("Choose File to Save Torrent"));

	if (s.isNull())
		return;
	
	if (!s.endsWith(".torrent"))
		s += ".torrent";

	KProgressDialog* dlg = new KProgressDialog(this,0);
	dlg->setLabel(i18n("Creating %1...").arg(s));
	dlg->setModal(true);
	dlg->setAllowCancel(false);
	dlg->show();
	core->makeTorrent(
			url,trackers,chunk_size,
			name,m_comments->text(),
			m_start_seeding->isChecked(),s,
			m_private->isChecked(),
			dlg->progressBar(),
			m_decentralized->isChecked());
	delete dlg;
	accept();
}

void TorrentCreatorDlg::errorMsg(const QString & text)
{
	KMessageBox::error(this,text,i18n("Error"));
}

void TorrentCreatorDlg::btnRemoveNode_clicked()
{
	QListViewItem* item = m_nodeList->currentItem();
	if(!item)
		return;
	
	m_nodeList->removeItem(item);
}

void TorrentCreatorDlg::btnAddNode_clicked()
{
	new QListViewItem(m_nodeList, m_node->text(), QString("%1").arg(m_port->value()));
}

void TorrentCreatorDlg::m_nodeList_selectionChanged(QListViewItem*)
{
	btnRemoveNode->setEnabled(m_nodeList->selectedItem()!=0);
}

void TorrentCreatorDlg::m_node_textChanged(const QString& txt)
{
	btnAddNode->setEnabled(!txt.isEmpty());
}

#include "torrentcreatordlg.moc"
