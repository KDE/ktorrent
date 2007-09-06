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
#include "core.h"
#include "torrentcreatordlg.h"

namespace kt
{
	TorrentCreatorDlg::TorrentCreatorDlg(Core* core,QWidget* parent) : QDialog(parent),core(core)
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
			
		}
	}
	
	void TorrentCreatorDlg::moveDownPressed()
	{
		QList<QListWidgetItem*> sel = m_tracker_list->selectedItems();
		foreach (QListWidgetItem* s,sel)
		{
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
		QDialog::accept();
	}
	
	void TorrentCreatorDlg::reject()
	{
		QDialog::reject();
	}
}

#include "torrentcreatordlg.moc"