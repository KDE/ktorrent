/***************************************************************************
 *   Copyright (C) 2006 by Ivan Vasić   								   *
 *   ivasic@gmail.com   												   *
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
#include "scanfolder.h"

#include <kdirlister.h>
#include <kfileitem.h>
#include <klocale.h>
#include <kio/job.h>

#include <qstring.h>
#include <qobject.h>
#include <qfile.h>

#include <torrent/globals.h>
#include <util/log.h>
#include <util/fileops.h>
#include <util/functions.h>
#include <util/constants.h>
		
#include <bcodec/bnode.h>
#include <bcodec/bdecoder.h>

#include <interfaces/coreinterface.h>
#include "scanfolderpluginsettings.h"

using namespace bt;

namespace kt
{

	ScanFolder::ScanFolder(CoreInterface* core, const QString& dir, LoadedTorrentAction action)
		: m_root_dir(dir),m_core(core), m_dir(0), m_loadedAction(action)
	{
		bt::Out(SYS_SNF|LOG_NOTICE) << "ScanFolder : scanning " << dir << endl;
		m_dir = new KDirLister();
		
		if (!m_root_dir.endsWith(bt::DirSeparator()))
			m_root_dir += bt::DirSeparator();

		if (!m_dir->openUrl(dir,KDirLister::Keep)) 
		{
			m_valid = false;
			bt::Out(SYS_SNF|LOG_NOTICE) << "ScanFolder : m_dir->openUrl failed " << endl;
			return;
		} 
		else
			m_valid = true;

		m_dir->setShowingDotFiles(true);

		connect(m_dir, SIGNAL(newItems( const KFileItemList& )), this, SLOT(onNewItems( const KFileItemList& )));
		connect(m_core, SIGNAL(loadingFinished( const KUrl&, bool, bool )), this, SLOT(onLoadingFinished( const KUrl&, bool, bool )));
		connect(&m_incomplePollingTimer,SIGNAL(timeout()),this,SLOT(onIncompletePollingTimeout()));
		
		if( (m_loadedAction == moveAction) && !QDir(dir + i18n("loaded")).exists())
			KIO::mkdir(dir + i18n("loaded"));
	}


	ScanFolder::~ScanFolder()
	{
		delete m_dir;
	}

	void ScanFolder::onNewItems(const KFileItemList& items)
	{
		bool rec = ScanFolderPluginSettings::recursive();
	
		foreach (const KFileItem &file, items)
		{
			QString name = file.name();
			QString filename = file.url().toLocalFile();
			
			if (file.isDir() && name != i18n("loaded") && rec)
			{
				if (!filename.endsWith(bt::DirSeparator()))
					filename += bt::DirSeparator();
				
				// make sure we don't go into a recursive infinite loop
				if (filename != m_root_dir)
				{
					// watch subdirectories, but not the loaded directory
					m_dir->openUrl(file.url(),KDirLister::Keep); 
				}
				continue;
			}

			if (!name.endsWith(".torrent"))
				continue;
			
			QString dirname = QFileInfo(filename).absolutePath();
			if (!dirname.endsWith(bt::DirSeparator()))
				dirname += bt::DirSeparator();
			
			if (!rec && m_root_dir != dirname) // if recusive is disabled root_dir must be equal to dirnam
				continue;

			if (name.startsWith("."))
			{
				//Check if corresponding torrent exists
				if(!QFile::exists(dirname + name.right(name.length() - 1)) && (m_loadedAction == defaultAction))
					QFile::remove(filename);

				continue;
			}

			KUrl source(filename);

			//If torrent has it's hidden complement - skip it.
			if (QFile::exists(dirname + "/." + name))
				continue;
			
			if (incomplete(source))
			{
				// incomplete file, try this again in 10 seconds
				bt::Out(SYS_SNF|LOG_NOTICE) << "ScanFolder : incomplete file " << source << endl;
				m_incompleteURLs.append(source);
				if (m_incompleteURLs.count() == 1)
				{
					// first URL so start the poll timer
					// lets poll every 10 seconds
					m_incomplePollingTimer.start(10000);
				}
			}
			else
			{
				bt::Out(SYS_SNF|LOG_NOTICE) << "ScanFolder : found " << source << endl;
				//Add pending entry...
				m_pendingURLs.push_back(source);
				
				QString group;
				if (ScanFolderPluginSettings::addToGroup())
					group = ScanFolderPluginSettings::group();
				//Load torrent
				if (ScanFolderPluginSettings::openSilently())
					m_core->loadSilently(source,group);
				else
					m_core->load(source,group);
			}
		}
	}
	
	void ScanFolder::onLoadingFinished(const KUrl & url, bool success, bool canceled)
	{
		Q_UNUSED(canceled);
		if(m_pendingURLs.empty() || !success)
			return;
		
		// if no entry is found than this torrent was not started by this plugin so - quit
		if (!m_pendingURLs.contains(url))
			return;
		
		//remove this entry
		m_pendingURLs.removeAll(url);
		
		QString name = url.fileName();
		QString dirname = QFileInfo(url.toLocalFile()).absolutePath();
		if (!dirname.endsWith(bt::DirSeparator()))
			dirname += bt::DirSeparator();
		
		QString filename = dirname + name;
		
		KUrl destination(m_root_dir + i18n("loaded") + bt::DirSeparator() + name);
		
		switch(m_loadedAction) 
		{
			case deleteAction:
					//If torrent has it's hidden complement - remove it too.
				if(QFile::exists(dirname + "." + name))
					QFile::remove(dirname + "." + name);
					// 				Out() << "Deleting: " << name.ascii() << endl;
				QFile::remove(filename);
				break;
			case moveAction:
					// 				Out() << "Moving: " << name.ascii() << endl;
					//If torrent has it's hidden complement - remove it too.
				if(QFile::exists(dirname + "." + name))
					QFile::remove(dirname + "." + name);

				// NetAccess considered harmfull !!!
				KIO::file_move(url, destination);
				break;
			case defaultAction:
				QFile f(dirname + "." + name);
				f.open(QIODevice::WriteOnly);
				f.close();
				break;
		}
	}

	void ScanFolder::setLoadedAction(const LoadedTorrentAction& theValue)
	{
		m_loadedAction = theValue;

		QString path = m_dir->url().toLocalFile();
		if (!path.endsWith(bt::DirSeparator()))
			path += bt::DirSeparator();
		
		QDir tmp(path);

		if( (m_loadedAction == moveAction) && !QDir(path + i18n("loaded")).exists())
			KIO::mkdir(path + i18n("loaded"));
	}

	void ScanFolder::setFolderUrl(QString& url)
	{
		m_valid = m_dir->openUrl(url);
	}
	
	bool ScanFolder::incomplete(const KUrl & src)
	{
		// try to decode file, if it is syntactically correct, we can try to load it
		QFile fptr(src.toLocalFile());
		if (!fptr.open(QIODevice::ReadOnly))
			return false;
		
		try
		{
			QByteArray data = fptr.readAll();
			
			bt::BDecoder dec(data,false);
			bt::BNode* n = dec.decode();
			if (n)
			{
				// valid node, so file is complete
				delete n;
				return false;
			}
			else
			{
				// decoding failed so incomplete
				return true;
			}
		}
		catch (...)
		{
			// any error means shit happened and the file is incomplete
			return true;
		}
		return false;
	}
	
	void ScanFolder::onIncompletePollingTimeout()
	{
		QMap<KUrl,QString> todo;
		bt::Out(SYS_SNF|LOG_NOTICE) << "ScanFolder : checking incomplete files" << endl; 
		for (QList<KUrl>::iterator i = m_incompleteURLs.begin(); i != m_incompleteURLs.end();)
		{
			KUrl source = *i;
			if (!bt::Exists(source.toLocalFile()))
			{
				// doesn't exist anymore, so throw out of list
				i = m_incompleteURLs.erase(i);
			}
			else if (!incomplete(source))
			{
				bt::Out(SYS_SNF|LOG_NOTICE) << "ScanFolder : incomplete file " << source << " appears to be completed " << endl;
				//Add pending entry...
				m_pendingURLs.push_back(source);
				
				QString group;
				if (ScanFolderPluginSettings::addToGroup())
					group = ScanFolderPluginSettings::group();
				
				// don't load directly to avoid nested eventloops
				todo.insert(source,group);
				// remove from incomplete list
				i = m_incompleteURLs.erase(i);
			}
			else
			{
				bt::Out(SYS_SNF|LOG_NOTICE) << "ScanFolder : still incomplete : " << source << endl;
				i++;
			}
		}
		
		// stop timer when no incomple URL's are left
		if (m_incompleteURLs.count() == 0)
			m_incomplePollingTimer.stop();
		
		QMap<KUrl,QString>::iterator i = todo.begin();
		while (i != todo.end())
		{
			if (ScanFolderPluginSettings::openSilently())
				m_core->loadSilently(i.key(),i.value());
			else
				m_core->load(i.key(),i.value());
			i++;
		}
	}
}

#include "scanfolder.moc"
