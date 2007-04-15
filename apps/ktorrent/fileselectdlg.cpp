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

#include <qlabel.h>
#include <qstring.h>
#include <qbuttongroup.h>
#include <qcombobox.h>
#include <qcheckbox.h>

#include <interfaces/torrentfileinterface.h>
#include <interfaces/torrentinterface.h>
#include "fileselectdlg.h"
#include <interfaces/filetreediritem.h>
#include <interfaces/filetreeitem.h>
#include <interfaces/functions.h>
#include <settings.h>
#include <util/functions.h>

#include <groups/group.h>
#include <groups/groupmanager.h>

#ifdef Q_OS_BSD4
#include <sys/param.h>
#include <sys/mount.h>
#else
#include <sys/vfs.h>
#endif

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
		file.emitDownloadStatusChanged();
	}
	
	//Setup custom download location
	QString dn = m_downloadLocation->url();
	
	if(!dn.endsWith(bt::DirSeparator()))
		dn += bt::DirSeparator();
	
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

	struct statfs stfs;
	statfs(tc->getDataDir().ascii(), &stfs);
	unsigned long long bytes_free = ((unsigned long long)stfs.f_bavail) *
									((unsigned long long)stfs.f_bsize);

	unsigned long long bytes_to_download = 0;
	bytes_to_download = root->bytesToDownload();

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
	root = new kt::FileTreeDirItem(m_file_view, tc->getStats().torrent_name, this);

	for (Uint32 i = 0;i < tc->getNumFiles();i++)
	{
		kt::TorrentFileInterface & file = tc->getTorrentFile(i);
		file.setEmitDownloadStatusChanged(false);
		root->insert(file.getPath(), file);
	}

	root->setOpen(true);

	//m_file_view->setRootIsDecorated(true);

	updateSizeLabels();
	
	pnlFiles->setEnabled(FALSE);
}

void FileSelectDlg::populateFields()
{
	m_downloadLocation->setURL(Settings::saveDir());
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

