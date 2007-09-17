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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#include <klocale.h>
#include <kmessagebox.h>
#include <kiconloader.h>
#include <kmimetype.h>
#include <qtreewidget.h>
#include <kstandardguiitem.h>
#include <kpushbutton.h>
#include <interfaces/torrentfileinterface.h>
#include <interfaces/torrentinterface.h>
#include <interfaces/filetreediritem.h>
#include <interfaces/filetreeitem.h>
#include <util/functions.h>
#include <util/fileops.h>
#include <interfaces/functions.h>
#include <groups/group.h>
#include <groups/groupmanager.h>
#include "fileselectdlg.h"
#include "settings.h"

using namespace bt;

namespace kt
{

	FileSelectDlg::FileSelectDlg(kt::GroupManager* gman,QWidget* parent) : QDialog(parent,Qt::Dialog),gman(gman)
	{
		setupUi(this);
		root = 0;
		connect(m_select_all,SIGNAL(clicked()),this,SLOT(selectAll()));
		connect(m_select_none,SIGNAL(clicked()),this,SLOT(selectNone()));
		connect(m_invert_selection,SIGNAL(clicked()),this,SLOT(invertSelection()));
		connect(m_ok,SIGNAL(clicked()),this,SLOT(accept()));
		connect(m_cancel,SIGNAL(clicked()),this,SLOT(reject()));
		connect(m_downloadLocation, SIGNAL(textChanged (const QString &)), this, SLOT(updateSizeLabels()));

		m_ok->setGuiItem(KStandardGuiItem::ok());
		m_cancel->setGuiItem(KStandardGuiItem::cancel());
	}

	FileSelectDlg::~FileSelectDlg()
	{}

	int FileSelectDlg::execute(kt::TorrentInterface* tc,bool* user, bool* start)
	{
		this->tc = tc;
		this->user = user;
		this->start = start;
		if (tc)
		{
			populateFields();
			m_file_view->clear();

			if (tc->getStats().multi_file_torrent)
			{
				root = new kt::FileTreeDirItem(m_file_view,tc->getStats().torrent_name,this);
				for (Uint32 i = 0;i < tc->getNumFiles();i++)
				{
					kt::TorrentFileInterface & file = tc->getTorrentFile(i);
					file.setEmitDownloadStatusChanged(false);
					root->insert(file.getPath(),file);
				}
				root->setExpanded(true);
			}
			else
			{
				QTreeWidgetItem* twi = new QTreeWidgetItem(m_file_view);
				twi->setText(0,tc->getStats().torrent_name);
				twi->setText(1,BytesToString(tc->getStats().total_bytes));
				twi->setIcon(0,SmallIcon(KMimeType::findByPath(tc->getStats().torrent_name)->iconName()));
				m_select_all->setEnabled(false);
				m_select_none->setEnabled(false);
				m_invert_selection->setEnabled(false);
			}
			m_file_view->setRootIsDecorated(true);
			m_file_view->resizeColumnToContents(0);
			m_file_view->resizeColumnToContents(1);
			return exec();
		}
		return QDialog::Rejected;
	}

	void FileSelectDlg::reject()
	{
		QDialog::reject();
	}

	void FileSelectDlg::accept()
	{
		QStringList pe_ex;
		for (Uint32 i = 0;i < tc->getNumFiles();i++)
		{
			kt::TorrentFileInterface & file = tc->getTorrentFile(i);
			if (file.doNotDownload() && file.isPreExistingFile())
			{
				// we have excluded a preexsting file
				pe_ex.append(file.getPath());
			}
		}
		
		if (pe_ex.count() > 0)
		{
			QString msg = i18n("You have deselected the following existing files. "
					"You will lose all data in these files, are you sure you want to do this ?");
			// better ask the user if (s)he wants to delete the already existing data
			int ret = KMessageBox::warningYesNoList(0,msg,pe_ex,QString::null,
								KGuiItem(i18n("Yes, delete the files")),
								KGuiItem(i18n("No, keep the files")));
			if (ret == KMessageBox::No)
			{
				for (Uint32 i = 0;i < tc->getNumFiles();i++)
				{
					kt::TorrentFileInterface & file = tc->getTorrentFile(i);
					if (file.doNotDownload() && file.isPreExistingFile())
						file.setDoNotDownload(false);
				}
			}
		}
		
		for (Uint32 i = 0;i < tc->getNumFiles();i++)
		{
			kt::TorrentFileInterface & file = tc->getTorrentFile(i);
			file.setEmitDownloadStatusChanged(true);
		}

		//Setup custom download location
		QString dn = m_downloadLocation->url().path();
		QString ddir = tc->getDataDir();
		if (!dn.endsWith(bt::DirSeparator()))
			dn += bt::DirSeparator();
		if (!ddir.endsWith(bt::DirSeparator()))
			ddir += bt::DirSeparator();

		if (dn != ddir)
			tc->changeOutputDir(dn, false);

		//Make it user controlled if needed
		*user = m_chkUserControlled->isChecked();
		*start = m_chkUserControlled->isChecked() && m_chkStartTorrent->isChecked();
		
		//Now add torrent to selected group
		if (m_cmbGroups->currentIndex() > 0)
		{
			QString groupName = m_cmbGroups->currentText();
			
			Group* group = gman->find(groupName);
			if (group)
			{
				group->addTorrent(tc);	
			}
		}

		// update the last save directory
		Settings::setLastSaveDir(dn);
		QDialog::accept();
	}

	void FileSelectDlg::selectAll()
	{
		if (root)
			root->setAllChecked(true);
	}

	void FileSelectDlg::selectNone()
	{
		if (root)
			root->setAllChecked(false);
	}

	void FileSelectDlg::invertSelection()
	{
		if (root)
			root->invertChecked();
	}

	void FileSelectDlg::populateFields()
	{
		QString dir = Settings::saveDir().path();
		if (!Settings::useSaveDir() || dir.isNull())
		{
			dir = Settings::lastSaveDir();
			if (dir.isNull())
				dir = QDir::homePath();
		}
		
		m_downloadLocation->setUrl(dir);
		updateSizeLabels();
		loadGroups();
	}

	void FileSelectDlg::loadGroups()
	{
		GroupManager::iterator it = gman->begin();
		
		QStringList grps;
		
		//First default group
		grps << i18n("All Torrents");
		
		//now custom ones
		while(it != gman->end())
		{
			grps << it->first;		
			++it;
		}
		
		m_cmbGroups->addItems(grps);
	}

	void FileSelectDlg::treeItemChanged()
	{
		updateSizeLabels();
	}

	void FileSelectDlg::updateSizeLabels()
	{
		//calculate free disk space

		KUrl sdir = KUrl(m_downloadLocation -> url());
		while( sdir.isValid() && sdir.isLocalFile() && (!sdir.isEmpty())  && (! QDir(sdir.path()).exists()) ) 
		{
			sdir = sdir.upUrl();
		}
		
		Uint64 bytes_free = 0;
		if (!FreeDiskSpace(sdir.path(),bytes_free))
		{
			FreeDiskSpace(tc->getDataDir(),bytes_free);
		}
		
		Uint64 bytes_to_download = 0;
		if (root)
			bytes_to_download = root->bytesToDownload();
		else
			bytes_to_download = tc->getStats().total_bytes;

		lblFree->setText(kt::BytesToString(bytes_free));
		lblRequired->setText(kt::BytesToString(bytes_to_download));

		if (bytes_to_download > bytes_free)
			lblStatus->setText("<font color=\"#ff0000\">" + i18n("%1 short!", kt::BytesToString(-1*(long long)(bytes_free - bytes_to_download))));
		else
			lblStatus->setText(kt::BytesToString(bytes_free - bytes_to_download));
	}
}

#include "fileselectdlg.moc"

