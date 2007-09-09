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
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kprogressdialog.h>
#include <dht/dht.h>
#include <torrent/globals.h>
#include "core.h"
#include "gui.h"
#include "torrentcreatordlg.h"

namespace kt
{
	TorrentCreatorDlg::TorrentCreatorDlg(Core* core,GUI* gui,QWidget* parent) : QDialog(parent),core(core),gui(gui)
	{
		setupUi(this);
		
		m_url->setMode(KFile::File | KFile::ExistingOnly | KFile::LocalOnly | KFile::Directory);
		
		// hide DHT box until m_dht is checked
		m_dht_box->hide();
		adjustSize();
		connect(m_dht,SIGNAL(toggled(bool)),this,SLOT(dhtToggled(bool)));
		
		// tracker box stuff
		connect(m_add_tracker,SIGNAL(clicked()),this,SLOT(addTrackerPressed()));
		connect(m_tracker,SIGNAL(returnPressed()),this,SLOT(addTrackerPressed()));
		connect(m_remove_tracker,SIGNAL(clicked()),this,SLOT(removeTrackerPressed()));
		connect(m_move_up,SIGNAL(clicked()),this,SLOT(moveUpPressed()));
		connect(m_move_down,SIGNAL(clicked()),this,SLOT(moveDownPressed()));
		connect(m_tracker,SIGNAL(textChanged(const QString&)),this,SLOT(trackerTextChanged(const QString &)));
		connect(m_tracker_list,SIGNAL(itemSelectionChanged()),this,SLOT(trackerSelectionChanged()));
		m_add_tracker->setEnabled(false); // disable until there is text in m_tracker
		m_remove_tracker->setEnabled(false);
		m_move_up->setEnabled(false);
		m_move_down->setEnabled(false);
		
		
		// dht box
		connect(m_add_node,SIGNAL(clicked()),this,SLOT(addNodePressed()));
		connect(m_node,SIGNAL(returnPressed()),this,SLOT(addNodePressed()));
		connect(m_remove_node,SIGNAL(clicked()),this,SLOT(removeNodePressed()));
		connect(m_node,SIGNAL(textChanged(const QString&)),this,SLOT(nodeTextChanged(const QString &)));
		connect(m_node_list,SIGNAL(itemSelectionChanged()),this,SLOT(nodeSelectionChanged()));
		m_add_node->setEnabled(false);
		m_remove_node->setEnabled(false);
		
		// populate dht box with some nodes from our own table
		QMap<QString, int> n = bt::Globals::instance().getDHT().getClosestGoodNodes(10);

		for(QMap<QString, int>::iterator it = n.begin(); it!=n.end(); ++it)
		{
			QTreeWidgetItem* twi = new QTreeWidgetItem(m_node_list);
			twi->setText(0,it.key());
			twi->setText(1,QString::number(it.value()));
			m_node_list->addTopLevelItem(twi);
		}
	}
	
	TorrentCreatorDlg::~TorrentCreatorDlg()
	{
	}
		
	
	void TorrentCreatorDlg::addTrackerPressed()
	{
		if (m_tracker->text().length() > 0)
		{
			m_tracker_list->addItem(m_tracker->text());
			m_tracker->clear();
		}
	}
	
	void TorrentCreatorDlg::removeTrackerPressed()
	{
		QList<QListWidgetItem*> sel = m_tracker_list->selectedItems();
		foreach (QListWidgetItem* s,sel)
		{
			delete s;
		}
	}
	
	void TorrentCreatorDlg::moveUpPressed()
	{
		QList<QListWidgetItem*> sel = m_tracker_list->selectedItems();
		foreach (QListWidgetItem* s,sel)
		{
			int r = m_tracker_list->row(s);
			if (r > 0)
			{
				m_tracker_list->insertItem(r - 1,m_tracker_list->takeItem(r));
				m_tracker_list->setCurrentRow(r - 1);
			}
		}
	}
	
	void TorrentCreatorDlg::moveDownPressed()
	{
		QList<QListWidgetItem*> sel = m_tracker_list->selectedItems();
		foreach (QListWidgetItem* s,sel)
		{
			int r = m_tracker_list->row(s);
			if (r + 1 < m_tracker_list->count())
			{
				m_tracker_list->insertItem(r + 1,m_tracker_list->takeItem(r));
				m_tracker_list->setCurrentRow(r + 1);
			}
		}
	}
		
	void TorrentCreatorDlg::addNodePressed()
	{
		if (m_node->text().length() > 0)
		{
			QTreeWidgetItem* twi = new QTreeWidgetItem(m_node_list);
			twi->setText(0,m_node->text());
			twi->setText(1,QString::number(m_port->value()));
			m_node_list->addTopLevelItem(twi);
			m_node->clear();
		}
	}
	
	void TorrentCreatorDlg::removeNodePressed()
	{
		QList<QTreeWidgetItem*> sel = m_node_list->selectedItems();
		foreach (QTreeWidgetItem* s,sel)
			delete s;
	}
		
	void TorrentCreatorDlg::dhtToggled(bool on)
	{
		m_dht_box->setShown(on);
		m_trackers_box->setShown(!on);
		m_private->setEnabled(!on);
	}
		
	void TorrentCreatorDlg::nodeTextChanged(const QString & str)
	{
		m_add_node->setEnabled(str.length() > 0);
	}
	
	void TorrentCreatorDlg::nodeSelectionChanged()
	{
		m_remove_node->setEnabled(m_node_list->selectedItems().count() > 0);
	}
		
	void TorrentCreatorDlg::trackerTextChanged(const QString & str)
	{
		m_add_tracker->setEnabled(str.length() > 0);
	}
	
	void TorrentCreatorDlg::trackerSelectionChanged()
	{
		bool enable_buttons = m_tracker_list->selectedItems().count() > 0;
		m_remove_tracker->setEnabled(enable_buttons);
		m_move_up->setEnabled(enable_buttons);
		m_move_down->setEnabled(enable_buttons);
	}
		
	void TorrentCreatorDlg::accept()
	{
		if (!m_url->url().isValid())
		{
			gui->errorMsg(i18n("You must select a file or a folder."));
			return;
		}

		if (m_tracker_list->count() == 0 && !m_dht->isChecked())
		{
			QString msg = i18n("You have not added a tracker, "
					"are you sure you want to create this torrent ?");
			if (KMessageBox::warningYesNo(gui,msg) == KMessageBox::No)
				return;
		}
	
		if (m_node_list->topLevelItemCount() == 0 && m_dht->isChecked())
		{
			gui->errorMsg(i18n("You must add at least one node."));
			return;
		}

		KUrl url = m_url->url();
		Uint32 chunk_size_table[] = 
		{
			32,64,128,256,512,1024,2048,4096,8192
		};
		
		int chunk_size = chunk_size_table[m_chunk_size->currentIndex()];
		QString name = url.fileName();
	
		QStringList trackers; 
	
		if (m_dht->isChecked())
		{
			for (int i = 0;i < m_node_list->topLevelItemCount(); ++i)
			{
				QTreeWidgetItem* item = m_node_list->topLevelItem(i);
				trackers.append(item->text(0) + "," +  item->text(1));
			}
		}
		else
		{
			for (int i = 0;i < m_tracker_list->count(); ++i)
			{
				QListWidgetItem* item = m_tracker_list->item(i);
				trackers.append(item->text());
			}
		}

		QString s = KFileDialog::getSaveFileName(
				KUrl(),"*.torrent|" + i18n("Torrent Files (*.torrent)"),
				this,i18n("Choose a file to save the torrent"));

		if (s.isNull())
			return;
	
		if (!s.endsWith(".torrent"))
			s += ".torrent";

		KProgressDialog* dlg = new KProgressDialog(this,0);
		dlg->setLabelText(i18n("Creating %1...",s));
		dlg->setModal(true);
		dlg->setAllowCancel(false);
		dlg->show();
		core->makeTorrent(
			url.path(),trackers,chunk_size,name,m_comments->text(),
			m_start_seeding->isChecked(),s,m_private->isChecked(),
			dlg->progressBar(),
			m_dht->isChecked());
		delete dlg;
		QDialog::accept();
	}
	
	void TorrentCreatorDlg::reject()
	{
		QDialog::reject();
	}
}

#include "torrentcreatordlg.moc"
