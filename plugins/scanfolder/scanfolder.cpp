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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "scanfolder.h"

#include <kdirlister.h>
#include <kfileitem.h>
#include <klocale.h>
#include <kio/netaccess.h>

#include <qstring.h>
#include <qobject.h>
#include <qfile.h>

#include <torrent/globals.h>
#include <util/log.h>
#include <util/constants.h>

#include <interfaces/coreinterface.h>

using namespace bt;

namespace kt
{

	ScanFolder::ScanFolder(CoreInterface* core, QString& dir, LoadedTorrentAction action, bool openSilently)
			: m_core(core), m_dir(0), m_loadedAction(action), m_openSilently(openSilently)
	{
		m_dir = new KDirLister();

		if(!m_dir->openURL(dir)) {
			m_valid = false;
			return;
		} else
			m_valid = true;

		m_dir->setShowingDotFiles(true);

		connect(m_dir, SIGNAL(newItems( const KFileItemList& )), this, SLOT(onNewItems( const KFileItemList& )));

// 		Out() << "LOADING SCANFOLDER: " << m_dir->url().path() << endl;
	}


	ScanFolder::~ScanFolder()
	{
// 		Out() << "UNLOADING SCANFOLDER: " << m_dir->url().path() << endl;
		delete m_dir;
	}

	void ScanFolder::onNewItems(const KFileItemList& items)
	{
		KFileItemList list = items;
		KFileItem* file;
		for(file=list.first(); file; file=list.next()) {
			QString name = file->name();
			QString dirname = m_dir->url().path();
			QString filename = dirname + "/" + name;


			if(!name.endsWith(".torrent"))
				continue;

			if(name.startsWith(".")) 
			{
				//Check if corresponding torrent exists
				if(!QFile::exists(m_dir->url().path() + "/" + name.right(name.length() - 1)) && (m_loadedAction == defaultAction))
					QFile::remove(filename);

				continue;
			}

			KURL source(filename);
			KURL destination(dirname + "/" + i18n("loaded") + "/" + name);

			//If torrent has it's hidden complement - skip it.
			if(QFile::exists(dirname + "/." + name))
				continue;
			
			//Load torrent
// 			Out() << "SF Loading: " << name.ascii() << endl;
			if(m_openSilently)
				m_core->loadSilently(source);
			else
				m_core->load(source);

			switch(m_loadedAction) {
					case deleteAction:
					//If torrent has it's hidden complement - remove it too.
					if(QFile::exists(dirname + "/." + name))
						QFile::remove(dirname + "/." + name);
					// 				Out() << "Deleting: " << name.ascii() << endl;
					QFile::remove(filename);
					break;
					case moveAction:
					// 				Out() << "Moving: " << name.ascii() << endl;
					//If torrent has it's hidden complement - remove it too.
					if(QFile::exists(dirname + "/." + name))
						QFile::remove(dirname + "/." + name);

					KIO::NetAccess::move(source, destination);
					break;
					case defaultAction:
					QFile f(dirname + "/." + name);
					f.open(IO_WriteOnly);
					f.close();
					break;
			}
		}
	}

	void ScanFolder::setOpenSilently(bool theValue)
	{
		m_openSilently = theValue;
	}

	void ScanFolder::setLoadedAction(const LoadedTorrentAction& theValue)
	{
		m_loadedAction = theValue;

		QDir tmp(m_dir->url().path());

		if( (m_loadedAction == moveAction) && !tmp.exists(i18n("loaded"), false))
			tmp.mkdir(i18n("loaded"), false);
	}

	void ScanFolder::setFolderUrl(QString& url)
	{
		if(!m_dir->openURL(url)) {
			m_valid = false;
			return;
		} else
			m_valid = true;
	}
}
