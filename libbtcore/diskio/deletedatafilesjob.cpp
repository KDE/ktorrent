/***************************************************************************
 *   Copyright (C) 2008 by Joris Guisson and Ivan Vasic                    *
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
#include <QDir>
#include <kio/deletejob.h>
#include <kio/jobuidelegate.h>
#include <util/log.h>
#include <util/fileops.h>
#include <util/functions.h>
#include "deletedatafilesjob.h"

namespace bt
{
	
	

	DeleteDataFilesJob::DeleteDataFilesJob(const QString & base)
			: Job(true,0),base(base),directory_tree(0)
	{
	}


	DeleteDataFilesJob::~DeleteDataFilesJob()
	{
		delete directory_tree;
	}
	
	void DeleteDataFilesJob::addFile(const QString & file)
	{
		files.append(KUrl(file));
	}

	void DeleteDataFilesJob::addEmptyDirectoryCheck(const QString & fpath)
	{
		if (!directory_tree)
			directory_tree = new DirTree(base);
		
		directory_tree->insert(fpath);
	}
	
	void DeleteDataFilesJob::start()
	{
		active_job = KIO::del(files,KIO::HideProgressInfo);
		connect(active_job,SIGNAL(result(KJob*)),this,SLOT(onDeleteJobDone(KJob*)));
	}

	void DeleteDataFilesJob::onDeleteJobDone(KJob* j)
	{
		if (j != active_job)
			return;
		
		if (active_job->error())
			active_job->ui()->showErrorMessage();
		active_job = 0;
		
		if (directory_tree)
			directory_tree->doDeleteOnEmpty(base);
		
		setError(0);
		emitResult();
	}
	
	
	void DeleteDataFilesJob::kill(bool quietly)
	{
		Q_UNUSED(quietly);
	}

	
	DeleteDataFilesJob::DirTree::DirTree(const QString & name) : name(name)
	{
		subdirs.setAutoDelete(true);
	}
	
	DeleteDataFilesJob::DirTree::~DirTree()
	{
	}
	
	void DeleteDataFilesJob::DirTree::insert(const QString & fpath)
	{
		int i = fpath.indexOf(bt::DirSeparator());
		if (i == -1) // last part of fpath is a file, so we need to ignore that
			return;
		
		QString dn = fpath.left(i);
		DirTree* d = subdirs.find(dn);
		if (!d)
		{
			d = new DirTree(dn);
			subdirs.insert(dn,d);
		}
		
		d->insert(fpath.mid(i+1));
	}

	void DeleteDataFilesJob::DirTree::doDeleteOnEmpty(const QString & base)
	{
		bt::PtrMap<QString,DirTree>::iterator i = subdirs.begin();
		while (i != subdirs.end())
		{
			i->second->doDeleteOnEmpty(base + i->first + bt::DirSeparator());
			i++;
		}
		
		QDir dir(base);	
		QStringList el = dir.entryList(QDir::AllEntries|QDir::System|QDir::Hidden);
		el.removeAll(".");
		el.removeAll("..");
		if (el.count() == 0)
		{
				// no childern so delete the directory
			Out(SYS_DIO|LOG_DEBUG) << "Deleting empty directory : " << base << endl;
			bt::Delete(base,true);
		}
	}
}
