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
#include <kmessagebox.h>
#include <kio/job.h>
#include <kio/jobuidelegate.h>
#include <kio/jobclasses.h>
#include <kstandardguiitem.h>
#include <util/log.h>
#include <util/error.h>
#include <util/file.h>
#include <util/fileops.h>
#include <util/functions.h>
#include <torrent/globals.h>
#include <torrent/torrent.h>
#include <diskio/chunkmanager.h>
#include <interfaces/coreinterface.h>
#include <datachecker/singledatachecker.h>
#include <datachecker/multidatachecker.h>
#include <settings.h>
#include "importdialog.h"


using namespace bt;

namespace kt
{
	ImportDialog::ImportDialog(CoreInterface* core,QWidget* parent)
	: QDialog(parent),DataCheckerListener(false),core(core)
	{
		setupUi(this);
		KUrlRequester* r = m_torrent_url;
		r->setMode(KFile::File|KFile::LocalOnly);
		r->setFilter("*.torrent|" + i18n("Torrent files") + "\n*|" + i18n("All files"));

		r = m_data_url;
		r->setMode(KFile::File|KFile::Directory|KFile::LocalOnly);

		connect(m_import_btn,SIGNAL(clicked()),this,SLOT(onImport()));
		connect(m_cancel_btn,SIGNAL(clicked()),this,SLOT(reject()));
		m_progress->setEnabled(false);
		m_progress->setValue(0);
		m_cancel_btn->setGuiItem(KStandardGuiItem::cancel());
		m_import_btn->setIcon(KIcon("document-import"));
	}

	ImportDialog::~ImportDialog()
	{}

	void ImportDialog::progress(Uint32 num,Uint32 total)
	{
		m_progress->setMaximum(total);
		m_progress->setValue(num);
	}

	void ImportDialog::status(bt::Uint32 num_failed,bt::Uint32 num_found,bt::Uint32 num_downloaded,bt::Uint32 num_not_downloaded )
	{
		// don't care
		Q_UNUSED(num_failed);
		Q_UNUSED(num_found);
		Q_UNUSED(num_downloaded);
		Q_UNUSED(num_not_downloaded);
	}

	void ImportDialog::finished()
	{
		// only used for check in separate thread, so does not apply for the import plugin
	}

	void ImportDialog::import(Torrent & tor)
	{
		// get the urls
		KUrl tor_url = m_torrent_url->url();
		KUrl data_url = m_data_url->url();

		// now we need to check the data
		DataChecker* dc = 0;
		if (tor.isMultiFile())
		{
			dc = new MultiDataChecker();
			QString path = data_url.path();
			if (!path.endsWith(bt::DirSeparator()))
				path += bt::DirSeparator();

			for (Uint32 i = 0;i < tor.getNumFiles();i++)
			{
				bt::TorrentFile & tf = tor.getFile(i);
				tf.setPathOnDisk(path + tf.getPath());
			}
		}
		else
			dc = new SingleDataChecker();

		try
		{
			BitSet bs(tor.getNumChunks());
			bs.setAll(false);
			dc->setListener(this);
			dc->check(data_url.path(),tor,QString::null,bs);
		}
		catch (Error & e)
		{
			delete dc;
			KMessageBox::error(this,i18n("Cannot verify data : %1",e.toString()));
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
			writeIndex(tor_dir + "index",dc->getResult());

			// copy the torrent file
			bt::CopyFile(tor_url.prettyUrl(),tor_dir + "torrent");

			Uint64 imported = calcImportedBytes(dc->getResult(),tor);

			// make the cache
			if (tor.isMultiFile())
			{
				QList<Uint32> dnd_files;

				// first make tor_dir/dnd
				QString dnd_dir = tor_dir + "dnd" + bt::DirSeparator();
				if (!bt::Exists(dnd_dir))
					MakeDir(dnd_dir);

				QString ddir = data_url.path();
				if (!ddir.endsWith(bt::DirSeparator()))
					ddir += bt::DirSeparator();

				for (Uint32 i = 0;i < tor.getNumFiles();i++)
				{
					TorrentFile & tf = tor.getFile(i);
					makeDirs(dnd_dir,data_url,tf.getPath());
					tf.setPathOnDisk(ddir + tf.getPath());
				}

				saveFileMap(tor,tor_dir);

				QString durl = data_url.path();
				if (durl.endsWith(bt::DirSeparator()))
					durl = durl.left(durl.length() - 1);
				int ds = durl.lastIndexOf(bt::DirSeparator());
				if (durl.mid(ds+1) == tor.getNameSuggestion())
				{
					durl = durl.left(ds);
					saveStats(tor_dir + "stats",KUrl(durl),imported,false);
				}
				else
				{
					saveStats(tor_dir + "stats",KUrl(durl),imported,true);
				}
				saveFileInfo(tor_dir + "file_info",dnd_files);
			}
			else
			{
				// single file, just symlink the data_url to tor_dir/cache
				QString durl = data_url.path();
				int ds = durl.lastIndexOf(bt::DirSeparator());
				durl = durl.left(ds);
				saveStats(tor_dir + "stats",durl,imported,false);
				saveFileMap(tor_dir,data_url.path());
			}

			// everything went OK, so load the whole shabang and start downloading
			core->loadExistingTorrent(tor_dir);
		}
		catch (Error & e)
		{
			// delete tor_dir
			bt::Delete(tor_dir,true);
			delete dc;
			KMessageBox::error(this,e.toString());
			reject();
			return;
		}

		delete dc;
		accept();
	}

	void ImportDialog::onTorrentGetReult(KJob* j)
	{
		if (j->error())
		{
			((KIO::Job*)j)->ui()->showErrorMessage();
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
				KMessageBox::error(this,i18n("Cannot load the torrent file : %1",e.toString()));
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

		KUrl tor_url = m_torrent_url->url();
		if (!tor_url.isLocalFile())
		{
			// download the torrent file
			KIO::StoredTransferJob* j = KIO::storedGet(tor_url);
			connect(j,SIGNAL(result(KJob* )),this,SLOT(onTorrentGetReult(KJob*)));
		}
		else
		{
			Torrent tor;

			// try to load the torrent
			try
			{
				tor.load(tor_url.path(),false);
			}
			catch (Error & e)
			{
				KMessageBox::error(this,i18n("Cannot load the torrent file : %1",e.toString()));
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
			throw Error(i18n("Cannot open %1 : %2",file,fptr.errorString()));

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

	void ImportDialog::makeDirs(const QString & dnd_dir,const KUrl & data_url,const QString & fpath)
	{
		QStringList sl = fpath.split(bt::DirSeparator());

		// create all necessary subdirs
		QString otmp = data_url.path();
		if (!otmp.endsWith(bt::DirSeparator()))
			otmp += bt::DirSeparator();

		QString dtmp = dnd_dir;
		for (int i = 0;i < sl.count() - 1;i++)
		{
			otmp += sl[i];
			dtmp += sl[i];
			if (!bt::Exists(otmp))
				MakeDir(otmp);
			if (!bt::Exists(dtmp))
				MakeDir(dtmp);
			otmp += bt::DirSeparator();
			dtmp += bt::DirSeparator();
		}
	}

	void ImportDialog::saveStats(const QString & stats_file,const KUrl & data_url,Uint64 imported,bool custom_output_name)
	{
		QFile fptr(stats_file);
		if (!fptr.open(QIODevice::WriteOnly))
		{
			Out(SYS_GEN|LOG_IMPORTANT) << "Warning : can't create stats file" << endl;
			return;
		}

		QTextStream out(&fptr);
		out << "OUTPUTDIR=" << data_url.path() << ::endl;
		out << "UPLOADED=0" << ::endl;
		out << "RUNNING_TIME_DL=0" << ::endl;
		out << "RUNNING_TIME_UL=0" << ::endl;
		out << "PRIORITY=0" << ::endl;
		out << "AUTOSTART=1" << ::endl;
		if (Settings::maxRatio() > 0)
			out << QString("MAX_RATIO=%1").arg(Settings::maxRatio(),0,'f',2) << ::endl;
		out << QString("IMPORTED=%1").arg(imported) << ::endl;
		if (custom_output_name)
			out << "CUSTOM_OUTPUT_NAME=1" << endl;
	}

	Uint64 ImportDialog::calcImportedBytes(const bt::BitSet & chunks,const Torrent & tor)
	{
		Uint64 nb = 0;
		Uint64 ls = tor.getLastChunkSize();

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

	void ImportDialog::saveFileInfo(const QString & file_info_file,QList<Uint32> & dnd)
	{
		// saves which TorrentFile's do not need to be downloaded
		File fptr;
		if (!fptr.open(file_info_file,"wb"))
		{
			Out(SYS_GEN|LOG_IMPORTANT) << "Warning : Can't save chunk_info file : " << fptr.errorString() << endl;
			return;
		}

		;

		// first write the number of excluded ones
		Uint32 tmp = dnd.count();
		fptr.write(&tmp,sizeof(Uint32));
		// then all the excluded ones
		for (int i = 0;i < dnd.count();i++)
		{
			tmp = dnd[i];
			fptr.write(&tmp,sizeof(Uint32));
		}
		fptr.flush();
	}

	void ImportDialog::saveFileMap(const Torrent & tor,const QString & tor_dir)
	{
		QString file_map = tor_dir + "file_map";
		QFile fptr(file_map);
		if (!fptr.open(QIODevice::WriteOnly))
			throw Error(i18n("Failed to create %1 : %2",file_map,fptr.errorString()));

		QTextStream out(&fptr);

		Uint32 num = tor.getNumFiles();
		for (Uint32 i = 0;i < num;i++)
		{
			const TorrentFile & tf = tor.getFile(i);
			out << tf.getPathOnDisk() << ::endl;
		}
	}

	void ImportDialog::saveFileMap(const QString & tor_dir,const QString & ddir)
	{
		QString file_map = tor_dir + "file_map";
		QFile fptr(file_map);
		if (!fptr.open(QIODevice::WriteOnly))
			throw Error(i18n("Failed to create %1 : %2",file_map,fptr.errorString()));

		QTextStream out(&fptr);
		out << ddir << ::endl;
	}
}



#include "importdialog.moc"

