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
#include <kicon.h>
#include <kiconloader.h>
#include <kmimetype.h>
#include <kstandardguiitem.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <interfaces/torrentinterface.h>
#include "missingfilesdlg.h"

namespace kt
{

	MissingFilesDlg::MissingFilesDlg(const QString & text,const QStringList & missing,bt::TorrentInterface* tc,QWidget* parent) : QDialog(parent),ret(CANCEL),tc(tc)
	{
		setupUi(this);
		
		m_text->setText(text);
		connect(m_cancel,SIGNAL(clicked()),this,SLOT(cancelPressed()));
		connect(m_quit,SIGNAL(clicked()),this,SLOT(quitPressed()));
		connect(m_recreate,SIGNAL(clicked()),this,SLOT(recreatePressed()));
		connect(m_dnd,SIGNAL(clicked()),this,SLOT(dndPressed()));
		connect(m_select_new,SIGNAL(clicked()),this,SLOT(selectNewPressed()));
		
		m_quit->setGuiItem(KStandardGuiItem::quit());
		m_cancel->setGuiItem(KStandardGuiItem::cancel());
		
		foreach (const QString &s,missing)
		{
			QListWidgetItem* lwi = new QListWidgetItem(m_file_list);
			lwi->setText(s);
			lwi->setIcon(SmallIcon(KMimeType::findByPath(s)->iconName()));
		}
		
		m_dnd->setEnabled(tc->getStats().multi_file_torrent);
		// select new is only possible if all files are missing 
		m_select_new->setEnabled(!tc->getStats().multi_file_torrent || missing.count() == tc->getNumFiles());
	}


	MissingFilesDlg::~MissingFilesDlg()
	{}
	
	void MissingFilesDlg::quitPressed()
	{
		ret = QUIT;
		accept();
	}
	
	void MissingFilesDlg::dndPressed()
	{
		ret = DO_NOT_DOWNLOAD;
		accept();
	}
	
	void MissingFilesDlg::selectNewPressed()
	{
		if (tc->getStats().multi_file_torrent)
		{
			QString dir = KFileDialog::getExistingDirectory(KUrl("kfiledialog:///openTorrent"),
					this,i18n("Select the directory where the data now is."));
			if (dir.isNull())
				return;
			
			QString old = tc->getStats().output_path;
			tc->changeOutputDir(dir,bt::TorrentInterface::FULL_PATH);
			QStringList dummy;
			if (tc->hasMissingFiles(dummy))
			{
				if (dummy.count() == tc->getNumFiles())
					KMessageBox::error(this,i18n("The data files are not present in the location you selected !"));
				else
					KMessageBox::error(this,i18n("Not all files where found in the new location, some are still missing !"));
				
				tc->changeOutputDir(old,bt::TorrentInterface::FULL_PATH);
			}
			else
			{
				ret = NEW_LOCATION_SELECTED;
				accept();
			}
		}
		else
		{
			QString dir = KFileDialog::getExistingDirectory(KUrl("kfiledialog:///openTorrent"),
					this,i18n("Select the directory where the data now is."));
			if (dir.isNull())
				return;
			
			QString old = tc->getDataDir();
			tc->changeOutputDir(dir,0);
			QStringList dummy;
			if (tc->hasMissingFiles(dummy))
			{
				KMessageBox::error(this,i18n("The data file is not present in the location you selected !"));
				tc->changeOutputDir(old,0);
			}
			else
			{
				ret = NEW_LOCATION_SELECTED;
				accept();
			}
		}
	}
	
	void MissingFilesDlg::recreatePressed()
	{
		ret = RECREATE;
		accept();
	}
	
	void MissingFilesDlg::cancelPressed()
	{
		ret = CANCEL;
		accept();
	}
		
	MissingFilesDlg::ReturnCode MissingFilesDlg::execute()
	{
		exec();
		return ret;
	}
}

#include "missingfilesdlg.moc"
