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
#include <kurl.h>
#include <klocale.h>
#include <kprogress.h>
#include <kurlrequester.h>
#include <kpushbutton.h>
#include <kmessagebox.h>
#include <kio/job.h>
#include <kio/jobclasses.h>
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
#include <datachecker/singledatachecker.h>
#include <datachecker/multidatachecker.h>

using namespace bt;

namespace kt
{
	ImportDialog::ImportDialog(CoreInterface* core,QWidget* parent, const char* name, bool modal, WFlags fl)
	: ImportDlgBase(parent,name, modal,fl),DataCheckerListener(false),core(core)
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
	
	void ImportDialog::progress(Uint32 num,Uint32 total)
	{
		m_progress->setTotalSteps(total);
		m_progress->setProgress(num);
	}
	
	void ImportDialog::status(Uint32 ,Uint32 )
	{
		// don't care
	}
	
	void ImportDialog::finished()
	{
		// only used for check in separate thread, so does not apply for the import plugin
	}
	
	void ImportDialog::import(Torrent & tor)
	{
		// get the urls
		KURL tor_url = KURL::fromPathOrURL(m_torrent_url->url());
		KURL data_url = KURL::fromPathOrURL(m_data_url->url());
		
		// now we need to check the data
		DataChecker* dc = 0;
		if (tor.isMultiFile())
			dc = new MultiDataChecker();
		else
			dc = new SingleDataChecker();
		
		try
		{
			dc->setListener(this);
			dc->check(data_url.path(),tor,QString::null);
		}
		catch (Error & e)
		{
			delete dc;
			KMessageBox::error(this,i18n("Cannot verify data : %1").arg(e.toString()),i18n("Error"));
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
			bt::CopyFile(tor_url.prettyURL(),tor_dir + "torrent");
			
			Uint64 imported = calcImportedBytes(dc->getDownloaded(),tor);
			
			// make the cache
			if (tor.isMultiFile())
			{
				QValueList<Uint32> dnd_files;
				bool dnd = false;
				// first make tor_dir/cache/
				QString cache_dir = tor_dir + "cache" + bt::DirSeparator();
				QString dnd_dir = tor_dir + "dnd" + bt::DirSeparator();
				if (!bt::Exists(cache_dir))
					MakeDir(cache_dir);
				if (!bt::Exists(dnd_dir))
					MakeDir(dnd_dir);
				
				
				// make all sub symlinks
				for (Uint32 i = 0;i < tor.getNumFiles();i++)
				{
					linkTorFile(cache_dir,dnd_dir,data_url,tor.getFile(i).getPath(),dnd);
					if (dnd)
						dnd_files.append(i);
					dnd = false;
				}
				
				QString durl = data_url.path();
				if (durl.endsWith(bt::DirSeparator()))
					durl = durl.left(durl.length() - 1);
				int ds = durl.findRev(bt::DirSeparator());
				if (durl.mid(ds+1) == tor.getNameSuggestion())
				{
					durl = durl.left(ds);
					saveStats(tor_dir + "stats",KURL::fromPathOrURL(durl),imported,false);
				}
				else
				{
					saveStats(tor_dir + "stats",KURL::fromPathOrURL(durl),imported,true);
				}
				saveFileInfo(tor_dir + "file_info",dnd_files);
			}
			else
			{
				// single file, just symlink the data_url to tor_dir/cache
				bt::SymLink(data_url.path(),tor_dir + "cache");
				QString durl = data_url.path();
				int ds = durl.findRev(bt::DirSeparator());
				durl = durl.left(ds);
				saveStats(tor_dir + "stats",durl,imported,false);
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
	
	void ImportDialog::onTorrentGetReult(KIO::Job* j)
	{
		if (j->error())
		{
			j->showErrorDialog(this);
			reject();
		}
		else
		{
			KIO::StoredTransferJob* stj = (KIO::StoredTransferJob*)j;
			Torrent tor;
		
			// try to load the torrent
			try
			{
				tor.load(stj->data(),false);
			}
			catch (Error & e)
			{
				KMessageBox::error(this,i18n("Cannot load the torrent file : %1").arg(e.toString()),
								   i18n("Error"));
				reject();
				return;
			}
			import(tor);
		}
	}
	
	void ImportDialog::onImport()
	{
		m_progress->setEnabled(true);
		m_import_btn->setEnabled(false);
		m_cancel_btn->setEnabled(false);
		m_torrent_url->setEnabled(false);
		m_data_url->setEnabled(false);
		
		KURL tor_url = KURL::fromPathOrURL(m_torrent_url->url());
		if (!tor_url.isLocalFile())
		{
			// download the torrent file
			KIO::StoredTransferJob* j = KIO::storedGet(tor_url);
			connect(j,SIGNAL(result(KIO::Job* )),this,SLOT(onTorrentGetReult(KIO::Job*)));
		}
		else
		{
			KURL tor_url = KURL::fromPathOrURL(m_torrent_url->url());
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
			import(tor);
		}
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
	
	void ImportDialog::linkTorFile(const QString & cache_dir,const QString & dnd_dir,
								   const KURL & data_url,const QString & fpath,bool & dnd)
	{
		QStringList sl = QStringList::split(bt::DirSeparator(),fpath);

		// create all necessary subdirs
		QString ctmp = cache_dir;
		QString otmp = data_url.path();
		if (!otmp.endsWith(bt::DirSeparator()))
			otmp += bt::DirSeparator();
		
		QString dtmp = dnd_dir;
		for (Uint32 i = 0;i < sl.count() - 1;i++)
		{
			otmp += sl[i];
			ctmp += sl[i];
			dtmp += sl[i];
			// we need to make the same directory structure in the cache
			// as the output dir
			if (!bt::Exists(ctmp))
				MakeDir(ctmp);
			if (!bt::Exists(otmp))
				MakeDir(otmp);
			if (!bt::Exists(dtmp))
				MakeDir(dtmp);
			otmp += bt::DirSeparator();
			ctmp += bt::DirSeparator();
			dtmp += bt::DirSeparator();
		}

		QString dfile = otmp + sl.last();
		if (!bt::Exists(dfile))
		{
			// when we start the torrent the user will be asked what to do
		//	bt::SymLink(dfile,cache_dir + fpath);
			dnd = false;
		}
		else
		{
			// just symlink the existing file
			bt::SymLink(dfile,cache_dir + fpath);
			dnd = false;
		}
	}
	
	void ImportDialog::saveStats(const QString & stats_file,const KURL & data_url,Uint64 imported,bool custom_output_name)
	{
		QFile fptr(stats_file);
		if (!fptr.open(IO_WriteOnly))
		{
			Out(SYS_PFI|LOG_IMPORTANT) << "Warning : can't create stats file" << endl;
			return;
		}

		QTextStream out(&fptr);
		out << "OUTPUTDIR=" << data_url.path() << ::endl;
		out << "UPLOADED=0" << ::endl;
		out << "RUNNING_TIME_DL=0" << ::endl;
		out << "RUNNING_TIME_UL=0" << ::endl;
		out << "PRIORITY=0" << ::endl;
		out << "AUTOSTART=1" << ::endl;
		if (core->getGlobalMaxShareRatio() > 0)
			out << QString("MAX_RATIO=%1").arg(core->getGlobalMaxShareRatio(),0,'f',2) << ::endl;
		out << QString("IMPORTED=%1").arg(imported) << ::endl;
		if (custom_output_name)
			out << "CUSTOM_OUTPUT_NAME=1" << endl;
	}
	
	Uint64 ImportDialog::calcImportedBytes(const bt::BitSet & chunks,const Torrent & tor)
	{
		Uint64 nb = 0;
		Uint64 ls = tor.getFileLength() % tor.getChunkSize();
		if (ls == 0)
			ls = tor.getChunkSize();
		
		for (Uint32 i = 0;i < chunks.getNumBits();i++)
		{
			if (!chunks.get(i))
				continue;
			
			if (i == chunks.getNumBits() - 1)
				nb += ls;
			else
				nb += tor.getChunkSize();
		}
		return nb;
	}
	
	void ImportDialog::saveFileInfo(const QString & file_info_file,QValueList<Uint32> & dnd)
	{
		// saves which TorrentFile's do not need to be downloaded
		File fptr;
		if (!fptr.open(file_info_file,"wb"))
		{
			Out(SYS_PFI|LOG_IMPORTANT) << "Warning : Can't save chunk_info file : " << fptr.errorString() << endl;
			return;
		}

		;

		// first write the number of excluded ones
		Uint32 tmp = dnd.count();
		fptr.write(&tmp,sizeof(Uint32));
		// then all the excluded ones
		for (Uint32 i = 0;i < dnd.count();i++)
		{
			tmp = dnd[i];
			fptr.write(&tmp,sizeof(Uint32));
		}
		fptr.flush();
	}
}



#include "importdialog.moc"

