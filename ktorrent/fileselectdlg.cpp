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
#include <QTextCodec>
#include <klocale.h>
#include <kmessagebox.h>
#include <kiconloader.h>
#include <kmimetype.h>
#include <qtreewidget.h>
#include <kstandardguiitem.h>
#include <kpushbutton.h>
#include <interfaces/torrentfileinterface.h>
#include <interfaces/torrentinterface.h>
#include <util/functions.h>
#include <util/fileops.h>
#include <util/log.h>
#include <util/error.h>
#include <interfaces/functions.h>
#include <groups/group.h>
#include <groups/groupmanager.h>
#include <torrent/torrentfiletreemodel.h>
#include <torrent/torrentfilelistmodel.h>
#include "fileselectdlg.h"
#include "settings.h"

using namespace bt;

namespace kt
{

	FileSelectDlg::FileSelectDlg(kt::GroupManager* gman,const QString & group_hint,QWidget* parent) : KDialog(parent),gman(gman),initial_group(0)
	{
		setupUi(mainWidget());

		model = 0;
		//root = 0;
		connect(m_select_all,SIGNAL(clicked()),this,SLOT(selectAll()));
		connect(m_select_none,SIGNAL(clicked()),this,SLOT(selectNone()));
		connect(m_invert_selection,SIGNAL(clicked()),this,SLOT(invertSelection()));
		
		m_downloadLocation->setMode(KFile::File|KFile::Directory|KFile::ExistingOnly|KFile::LocalOnly);
		
		encodings = QTextCodec::availableMibs();
		foreach (int mib,encodings)
		{
			m_encoding->addItem(QTextCodec::codecForMib(mib)->name());
		}
		
		if (!group_hint.isNull())
			initial_group = gman->find(group_hint);
	}

	FileSelectDlg::~FileSelectDlg()
	{}

	int FileSelectDlg::execute(bt::TorrentInterface* tc,bool* user, bool* start,bool* skip_check)
	{
		this->tc = tc;
		this->user = user;
		this->start = start;
		this->skip_check = skip_check;
		if (tc)
		{
			int idx = encodings.indexOf(tc->getTextCodec()->mibEnum());
			Out(SYS_GEN|LOG_DEBUG) << "Codec: " << QString(tc->getTextCodec()->name()) << " " << idx << endl;
			m_encoding->setCurrentIndex(idx);
			connect(m_encoding,SIGNAL(currentIndexChanged(const QString &)),this,SLOT(onCodecChanged(const QString&)));
			
			for (Uint32 i = 0;i < tc->getNumFiles();i++)
			{
				bt::TorrentFileInterface & file = tc->getTorrentFile(i);
				file.setEmitDownloadStatusChanged(false);
			}
			
			populateFields();
			if (Settings::useFileList())
				model = new TorrentFileListModel(tc,TorrentFileTreeModel::DELETE_FILES,this);
			else
				model = new TorrentFileTreeModel(tc,TorrentFileTreeModel::DELETE_FILES,this);
			
			connect(model,SIGNAL(checkStateChanged()),this,SLOT(updateSizeLabels()));
			connect(m_downloadLocation, SIGNAL(textChanged (const QString &)), this, SLOT(updateSizeLabels()));
			m_file_view->setModel(model);
			m_file_view->expandAll();
			
			updateSizeLabels();

			if (!tc->getStats().multi_file_torrent)
			{
				m_select_all->setEnabled(false);
				m_select_none->setEnabled(false);
				m_invert_selection->setEnabled(false);
			}

			m_file_view->setAlternatingRowColors(false);
			m_file_view->setRootIsDecorated(tc->getStats().multi_file_torrent);
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
		
		QString dn = m_downloadLocation->url().path();
		if (!dn.endsWith(bt::DirSeparator()))
			dn += bt::DirSeparator();

		if (!bt::Exists(dn))
		{
			try
			{
				if (KMessageBox::questionYesNo(this,i18n("The directory %1 does not exist, do you want to create it ?",dn)) == KMessageBox::Yes)
					MakePath(dn);	
				else
					return;
			}
			catch (bt::Error & err)
			{
				KMessageBox::error(this,err.toString());
				QDialog::reject();
				return;
			}
		}
		
		QString tld = m_toplevel_directory->text().trimmed();
		if (tld.isNull() || tld.length() == 0)
			tld = tc->getStats().torrent_name;

		for (Uint32 i = 0;i < tc->getNumFiles();i++)
		{
			bt::TorrentFileInterface & file = tc->getTorrentFile(i);

			// check for preexisting files
			QString path = dn + tld + bt::DirSeparator() + file.getPath();
			if (bt::Exists(path))
				file.setPreExisting(true);

			if (file.doNotDownload() && file.isPreExistingFile())
			{
				// we have excluded a preexsting file
				pe_ex.append(file.getPath());
			}
			file.setPathOnDisk(path);
			file.setEmitDownloadStatusChanged(true);
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
					bt::TorrentFileInterface & file = tc->getTorrentFile(i);
					if (file.doNotDownload() && file.isPreExistingFile())
						file.setDoNotDownload(false);
				}
			}
		}
		
		for (Uint32 i = 0;i < tc->getNumFiles();i++)
		{
			bt::TorrentFileInterface & file = tc->getTorrentFile(i);
			file.setEmitDownloadStatusChanged(true);
		}

		//Setup custom download location
		QString ddir = tc->getDataDir();
		if (!ddir.endsWith(bt::DirSeparator()))
			ddir += bt::DirSeparator();

		if (tc->getStats().multi_file_torrent && tld != tc->getStats().torrent_name)
			tc->changeOutputDir(dn + tld,bt::TorrentInterface::FULL_PATH);
		else if (dn != ddir)
			tc->changeOutputDir(dn, 0);

		//Make it user controlled if needed
		*user = m_chkUserControlled->isChecked();
		*start = m_chkUserControlled->isChecked() && m_chkStartTorrent->isChecked();
		*skip_check = m_skip_data_check->isChecked();
		
		//Now add torrent to selected group
		if (m_cmbGroups->currentIndex() > 0)
		{
			QString groupName = m_cmbGroups->currentText();
			
			Group* group = gman->find(groupName);
			if (group)
			{
				group->addTorrent(tc,true);	
				gman->saveGroups();
			}
		}

		// update the last save directory
		Settings::setLastSaveDir(dn);
		QDialog::accept();
	}

	void FileSelectDlg::selectAll()
	{
		model->checkAll();
	}

	void FileSelectDlg::selectNone()
	{
		model->uncheckAll();
	}

	void FileSelectDlg::invertSelection()
	{
		model->invertCheck();
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
		m_toplevel_directory->setEnabled(tc->getStats().multi_file_torrent);
		if (tc->getStats().multi_file_torrent)
			m_toplevel_directory->setText(tc->getStats().torrent_name);
		loadGroups();
	}

	void FileSelectDlg::loadGroups()
	{
		GroupManager::iterator it = gman->begin();
		
		QStringList grps;
		
		//First default group
		grps << i18n("All Torrents");
		
		int cnt = 0;
		int selected = 0;
		//now custom ones
		while(it != gman->end())
		{
			grps << it->first;		
			if (it->second == initial_group)
				selected = cnt + 1;
			++it;
			cnt++;
		}
		
		m_cmbGroups->addItems(grps);
		connect(m_cmbGroups,SIGNAL(activated(int)),this,SLOT(groupActivated(int)));
		
		if (selected > 0 && initial_group)
		{
			m_cmbGroups->setCurrentIndex(selected);
			QString dir = initial_group->groupPolicy().default_save_location;
			if (!dir.isNull() && bt::Exists(dir))
				m_downloadLocation->setUrl(KUrl(dir));
		}
	}
	
	void FileSelectDlg::groupActivated(int idx)
	{
		if (idx == 0)
			return; // No group selected
		
		// find the selected group	
		Group* g = gman->find(m_cmbGroups->itemText(idx));
		if (!g)
			return;
			
		QString dir = g->groupPolicy().default_save_location;
		if (!dir.isNull() && bt::Exists(dir))
			m_downloadLocation->setUrl(KUrl(dir));
	}

	void FileSelectDlg::updateSizeLabels()
	{
		if (!model)
			return;
		
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
		
		Uint64 bytes_to_download = model->bytesToDownload();

		lblFree->setText(bt::BytesToString(bytes_free));
		lblRequired->setText(bt::BytesToString(bytes_to_download));

		if (bytes_to_download > bytes_free)
			lblStatus->setText("<font color=\"#ff0000\">" + i18n("%1 short!", bt::BytesToString(-1*(long long)(bytes_free - bytes_to_download))));
		else
			lblStatus->setText(bt::BytesToString(bytes_free - bytes_to_download));
	}
	
	void FileSelectDlg::onCodecChanged(const QString & text)
	{
		QTextCodec* codec = QTextCodec::codecForName(text.toLocal8Bit());
		if (codec)
		{
			tc->changeTextCodec(codec);
			model->onCodecChange();
		}
	}
}

#include "fileselectdlg.moc"

