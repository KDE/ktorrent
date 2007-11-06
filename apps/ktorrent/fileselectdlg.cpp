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
#include <klistview.h>
#include <kstdguiitem.h>
#include <kpushbutton.h>
#include <kurlrequester.h>
#include <kiconloader.h>
#include <kmimetype.h>
#include <kurl.h>

#include <qlabel.h>
#include <qstring.h>
#include <qbuttongroup.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qdir.h>

#include <interfaces/torrentfileinterface.h>
#include <interfaces/torrentinterface.h>
#include "fileselectdlg.h"
#include <interfaces/filetreediritem.h>
#include <interfaces/filetreeitem.h>
#include <interfaces/functions.h>
#include <settings.h>
#include <util/functions.h>
#include <util/fileops.h>

#include <groups/group.h>
#include <groups/groupmanager.h>

using namespace kt;

FileSelectDlg::FileSelectDlg(GroupManager* gm, bool* user, bool* start, QWidget* parent, const char* name, bool modal, WFlags fl)
		: FileSelectDlgBase(parent, name, modal, fl), m_gman(gm), m_user(user), m_start(start)
{
	root = 0;
	connect(m_select_all, SIGNAL(clicked()), this, SLOT(selectAll()));
	connect(m_select_none, SIGNAL(clicked()), this, SLOT(selectNone()));
	connect(m_invert_selection, SIGNAL(clicked()), this, SLOT(invertSelection()));
	connect(m_ok, SIGNAL(clicked()), this, SLOT(accept()));
	connect(m_cancel, SIGNAL(clicked()), this, SLOT(reject()));
	connect(m_downloadLocation, SIGNAL(textChanged (const QString &)), this, SLOT(updateSizeLabels()));
	
	m_downloadLocation->setMode(KFile::Directory);
}

FileSelectDlg::~FileSelectDlg()

{}

int FileSelectDlg::execute(kt::TorrentInterface* tc)
{
	this->tc = tc;

	if (tc)
	{
		populateFields();
		
		if(tc->getStats().multi_file_torrent)
			setupMultifileTorrent();
		else
			setupSinglefileTorrent();		
		
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
	
	QString dn = m_downloadLocation->url();
	if (!dn.endsWith(bt::DirSeparator()))
		dn += bt::DirSeparator();

	for (Uint32 i = 0;i < tc->getNumFiles();i++)
	{
		kt::TorrentFileInterface & file = tc->getTorrentFile(i);

		// check for preexsting files
		QString path = dn + tc->getStats().torrent_name + bt::DirSeparator() + file.getPath();
		if (bt::Exists(path))
			file.setPreExisting(true);
		
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
		int ret = KMessageBox::warningYesNoList(0, msg, pe_ex, QString::null,
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
		// we don't need to emit the downloadStatusChanged signal, 
		// because tc->createFiles() in ktorrentcore.cpp will take care of everything
	}
	
	//Setup custom download location
	QString ddir = tc->getDataDir();
	if (!ddir.endsWith(bt::DirSeparator()))
		ddir += bt::DirSeparator();
	
	if (dn != ddir)	// only change when absolutely necessary
		tc->changeOutputDir(dn, false);
	
	//Make it user controlled if needed
	*m_user = m_chkUserControlled->isChecked();
	*m_start = m_chkUserControlled->isChecked() && m_chkStartTorrent->isChecked();
	
	//Now add torrent to selected group
	if(m_cmbGroups->currentItem() != 0)
	{
		QString groupName = m_cmbGroups->currentText();
		
		Group* group = m_gman->find(groupName);
		if(group)
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

void FileSelectDlg::updateSizeLabels()
{
	//calculate free disk space

	KURL sdir = KURL(m_downloadLocation -> url());
	while( sdir.isValid() && sdir.isLocalFile() && (!sdir.isEmpty())  && (! QDir(sdir.path()).exists()) ) 
	{
		sdir = sdir.upURL();
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
		lblStatus->setText("<font color=\"#ff0000\">" + kt::BytesToString(-1*(long long)(bytes_free - bytes_to_download)) + i18n(" short!"));
	else
		lblStatus->setText(kt::BytesToString(bytes_free - bytes_to_download));
}

void FileSelectDlg::treeItemChanged()
{
	updateSizeLabels();
}

void FileSelectDlg::setupMultifileTorrent()
{
	m_file_view->clear();
	root = new kt::FileTreeDirItem(m_file_view, tc->getStats().torrent_name, this);

	for (Uint32 i = 0;i < tc->getNumFiles();i++)
	{
		kt::TorrentFileInterface & file = tc->getTorrentFile(i);
		file.setEmitDownloadStatusChanged(false);
		root->insert(file.getPath(), file);
	}

	root->setOpen(true);
	m_file_view->setRootIsDecorated(true);

	updateSizeLabels();
}

void FileSelectDlg::setupSinglefileTorrent()
{
	m_file_view->clear();
	KListViewItem* single_root = new KListViewItem(m_file_view);
	single_root->setText(0,tc->getStats().torrent_name);
	single_root->setText(1,BytesToString(tc->getStats().total_bytes));
	single_root->setText(2,i18n("Yes"));
	single_root->setPixmap(0,KMimeType::findByPath(tc->getStats().torrent_name)->pixmap(KIcon::Small));
	root = 0;
	updateSizeLabels();
	m_select_all->setEnabled(false);
	m_select_none->setEnabled(false);
	m_invert_selection->setEnabled(false);
}

void FileSelectDlg::populateFields()
{
	QString dir = Settings::saveDir();
	if (!Settings::useSaveDir() || dir.isNull())
	{
		dir = Settings::lastSaveDir();
		if (dir.isNull())
			dir = QDir::homeDirPath();
	}
	
	m_downloadLocation->setURL(dir);
	loadGroups();
}

void FileSelectDlg::loadGroups()
{
	GroupManager::iterator it = m_gman->begin();
	
	QStringList grps;
	
	//First default group
	grps << i18n("All Torrents");
	
	//now custom ones
	while(it != m_gman->end())
	{
		grps << it->first;		
		++it;
	}
	
	m_cmbGroups->insertStringList(grps);
}

#include "fileselectdlg.moc"

