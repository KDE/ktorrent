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

	DeleteDataFilesJob::DeleteDataFilesJob()
			: KIO::Job()
	{
	}


	DeleteDataFilesJob::~DeleteDataFilesJob()
	{
	}
	
	void DeleteDataFilesJob::addFile(const QString & file)
	{
		files.append(KUrl(file));
	}

	void DeleteDataFilesJob::addEmptyDirectoryCheck(const QString & base,const QString & fpath)
	{
		directory_checks.append(qMakePair(base,fpath));
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
		
		QList<QPair<QString,QString> >::iterator i = directory_checks.begin();
		while (i != directory_checks.end())
		{
			deleteEmptyDirs(i->first,i->second);
			i++;
		}
		
		setError(0);
		emitResult();
	}
	
	void DeleteDataFilesJob::deleteEmptyDirs(const QString & base,const QString & fpath)
	{
		QStringList sl = fpath.split(bt::DirSeparator());
		// remove the last, which is just the filename
		sl.pop_back();
		
		while (sl.count() > 0)
		{
			QString path = base;
			// reassemble the full directory path
			for (QStringList::iterator itr = sl.begin(); itr != sl.end();itr++)
				path += *itr + bt::DirSeparator();
			
			QDir dir(path);
			if (!dir.exists())
			{
				sl.pop_back(); // remove the last so we can go one higher
				continue;
			}
				
			QStringList el = dir.entryList(QDir::AllEntries|QDir::System|QDir::Hidden);
			el.removeAll(".");
			el.removeAll("..");
			if (el.count() == 0)
			{
				// no childern so delete the directory
				Out(SYS_DIO|LOG_DEBUG) << "Deleting empty directory : " << path << endl;
				bt::Delete(path,true);
				sl.pop_back(); // remove the last so we can go one higher
			}
			else
			{
				
				// children, so we cannot delete any more directories higher up
				return;
			}
		}
		
		// now the output_dir itself
		QDir dir(base);
		if (dir.exists())
		{
			QStringList el = dir.entryList(QDir::AllEntries|QDir::System|QDir::Hidden);
			el.removeAll(".");
			el.removeAll("..");
			if (el.count() == 0)
			{
				Out(SYS_DIO|LOG_DEBUG) << "Deleting empty directory : " << base << endl;
				bt::Delete(base,true);
			}
		}
	}

}
