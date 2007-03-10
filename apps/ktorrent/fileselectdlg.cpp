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

#include <qlabel.h>
#include <qstring.h>

#include <interfaces/torrentfileinterface.h>
#include <interfaces/torrentinterface.h>
#include "fileselectdlg.h"
#include <interfaces/filetreediritem.h>
#include <interfaces/filetreeitem.h>
#include <interfaces/functions.h>

#ifdef Q_OS_BSD4
#include <sys/param.h>
#include <sys/mount.h>
#else
#include <sys/vfs.h>
#endif

FileSelectDlg::FileSelectDlg(QWidget* parent, const char* name, bool modal, WFlags fl)
		: FileSelectDlgBase(parent,name, modal,fl)
{
	root = 0;
	connect(m_select_all,SIGNAL(clicked()),this,SLOT(selectAll()));
	connect(m_select_none,SIGNAL(clicked()),this,SLOT(selectNone()));
	connect(m_invert_selection,SIGNAL(clicked()),this,SLOT(invertSelection()));
	connect(m_ok,SIGNAL(clicked()),this,SLOT(accept()));
	connect(m_cancel,SIGNAL(clicked()),this,SLOT(reject()));
	m_ok->setGuiItem(KStdGuiItem::ok());
	m_cancel->setGuiItem(KStdGuiItem::cancel());
	
	shortDiskSpace = false;
}

FileSelectDlg::~FileSelectDlg()
{}

int FileSelectDlg::execute(kt::TorrentInterface* tc)
{
	this->tc = tc;
	if (tc)
	{
		m_file_view->clear();
		root = new kt::FileTreeDirItem(m_file_view,tc->getStats().torrent_name);
		for (Uint32 i = 0;i < tc->getNumFiles();i++)
		{
			kt::TorrentFileInterface & file = tc->getTorrentFile(i);
			file.setEmitDownloadStatusChanged(false);
			root->insert(file.getPath(),file);
		}
		root->setOpen(true);
		m_file_view->setRootIsDecorated(true);
		
		
		//calculate free disk space
		struct statfs stfs;		
		statfs(tc->getDataDir().ascii(), &stfs);		
		unsigned long long bytes_free = ((unsigned long long)stfs.f_bavail) * 
                 ((unsigned long long)stfs.f_bsize);		
		unsigned long long bytes_to_download = tc->getStats().total_bytes;		
		
		lblFree->setText(kt::BytesToString(bytes_free));		
		lblRequired->setText(kt::BytesToString(bytes_to_download));
				
		if (shortDiskSpace = bytes_to_download > bytes_free)
			lblStatus->setText("<font color=\"#ff0000\">" + kt::BytesToString(-1*(long long)(bytes_free - bytes_to_download)) + i18n(" short!"));
		else
			lblStatus->setText(kt::BytesToString(bytes_free - bytes_to_download));
				
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
	if(shortDiskSpace && KMessageBox::questionYesNo(this, i18n("You don't have enough disk space to download this torrent. Are you sure you want to continue?"), i18n("Insufficient disk space")) == KMessageBox::No)
		return;	

			
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
		file.emitDownloadStatusChanged();
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


#include "fileselectdlg.moc"

