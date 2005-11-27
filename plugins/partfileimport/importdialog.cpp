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
 *   51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#include <kurl.h>
#include <klocale.h>
#include <kprogress.h>
#include <kurlrequester.h>
#include <kpushbutton.h>
#include <kmessagebox.h>
#include <util/log.h>
#include <util/error.h>
#include <util/file.h>
#include <util/fileops.h>
#include <util/functions.h>
#include <torrent/globals.h>
#include <torrent/torrent.h>
#include <torrent/chunkmanager.h>
#include <interfaces/coreinterface.h>
#include "importdialog.h"
#include "singledatachecker.h"
#include "multidatachecker.h"

using namespace bt;

namespace kt
{
	ImportDialog::ImportDialog(CoreInterface* core,QWidget* parent, const char* name, bool modal, WFlags fl)
	: ImportDlgBase(parent,name, modal,fl),core(core)
	{
		KURLRequester* r = m_torrent_url;
		r->setMode(KFile::File|KFile::LocalOnly);
		r->setFilter("*.torrent|" + i18n("Torrent files") + "\n*|" + i18n("All files"));
		
		r = m_data_url;
		r->setMode(KFile::File|KFile::Directory|KFile::LocalOnly);
	
		connect(m_import_btn,SIGNAL(clicked()),this,SLOT(onImport()));
		connect(m_cancel_btn,SIGNAL(clicked()),this,SLOT(reject()));
		m_progress->setEnabled(false);
	}
	
	ImportDialog::~ImportDialog()
	{}
	
	void ImportDialog::onImport()
	{
		m_progress->setEnabled(true);
		m_import_btn->setEnabled(false);
		m_cancel_btn->setEnabled(false);
		m_torrent_url->setEnabled(false);
		m_data_url->setEnabled(false);
		
		// get the urls
		KURL tor_url = m_torrent_url->url();
		KURL data_url = m_data_url->url();
		Torrent tor;
		
		// try to load the torrent
		try
		{
			tor.load(tor_url.path(),false);
		}
		catch (Error & e)
		{
			KMessageBox::error(this,i18n("Cannot load the torrent file : %1").arg(e.toString()),
							   i18n("Error"));
			reject();
			return;
		}
		
		// now we need to check the data
		DataChecker* dc = 0;
		if (tor.isMultiFile())
			dc = new MultiDataChecker();
		else
			dc = new SingleDataChecker();
		
		try
		{
			dc->check(data_url.path(),tor,m_progress);
		}
		catch (Error & e)
		{
			delete dc;
			KMessageBox::error(this,i18n("Cannot verify data : %s").arg(e.toString()),i18n("Error"));
			reject();
			return;
		}
		
		// find a new torrent dir and make it if necessary
		QString tor_dir = core->findNewTorrentDir();
		if (!tor_dir.endsWith(bt::DirSeparator()))
			tor_dir += bt::DirSeparator();
		
		try
		{	
			if (!bt::Exists(tor_dir))
				bt::MakeDir(tor_dir);
			
			// write the index file
			writeIndex(tor_dir + "index",dc->getDownloaded());
			
			// copy the torrent file
			bt::CopyFile(tor_url,tor_dir + "torrent");
			
			// make the cache
			if (tor.isMultiFile())
			{
				// first make tor_dir/cache/
				QString cache_dir = tor_dir + "cache" + bt::DirSeparator();
				if (!bt::Exists(cache_dir))
					MakeDir(cache_dir);
				
				// make all sub symlinks
				for (Uint32 i = 0;i < tor.getNumFiles();i++)
				{
					linkTorFile(cache_dir,data_url,tor.getFile(i).getPath());
				}
			}
			else
			{
				// single file, just symlink the data_url to tor_dir/cache
				bt::SymLink(data_url.path(),tor_dir + "cache");
			}
			
			// everything went OK, so load the whole shabang and start downloading
			core->loadExistingTorrent(tor_dir);
		}
		catch (Error & e)
		{
			// delete tor_dir
			bt::Delete(tor_dir,true);
			delete dc;
			KMessageBox::error(this,e.toString(),i18n("Error"));
			reject();
			return;
		}
		
		delete dc;
		accept();
	}
	
	void ImportDialog::writeIndex(const QString & file,const BitSet & chunks)
	{
		// first try to open it
		File fptr;
		if (!fptr.open(file,"wb"))
			throw Error(i18n("Cannot open %1 : %2").arg(file).arg(fptr.errorString()));
		
		// write all chunks to the file
		for (Uint32 i = 0;i < chunks.getNumBits();i++)
		{
			if (!chunks.get(i))
				continue;
			
			// we have the chunk so write a NewChunkHeader struct to the file
			NewChunkHeader hdr;
			hdr.index = i;
			hdr.deprecated = 0;
			fptr.write(&hdr,sizeof(NewChunkHeader));
		}
	}
	
	void ImportDialog::linkTorFile(const QString & cache_dir,const KURL & data_url,const QString & fpath)
	{
		QStringList sl = QStringList::split(bt::DirSeparator(),fpath);

		// create all necessary subdirs
		QString ctmp = cache_dir;
		QString otmp = data_url.path();
		for (Uint32 i = 0;i < sl.count() - 1;i++)
		{
			otmp += sl[i];
			ctmp += sl[i];
			// we need to make the same directory structure in the cache
			// as the output dir
			if (!bt::Exists(ctmp))
				MakeDir(ctmp);
			if (!bt::Exists(otmp))
				MakeDir(otmp);
			otmp += bt::DirSeparator();
			ctmp += bt::DirSeparator();
		}

		QString dfile = otmp + sl.last();
		// then make the file if it doesn't exist
		if (!bt::Exists(dfile))
			bt::Touch(dfile);
		// and make a symlink in the cache to it
		bt::SymLink(dfile,cache_dir + fpath);
	}
}



#include "importdialog.moc"

