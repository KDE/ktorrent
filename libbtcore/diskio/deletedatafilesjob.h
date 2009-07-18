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
#ifndef BTDELETEDATAFILESJOB_H
#define BTDELETEDATAFILESJOB_H

#include <util/ptrmap.h>
#include <QString>
#include <torrent/job.h>
#include <kurl.h>

namespace bt
{

	/**
	 * @author Joris Guisson <joris.guisson@gmail.com>
	 * KIO::Job to delete all the files of a torrent.
	 */
	class DeleteDataFilesJob : public Job
	{
		Q_OBJECT
	public:
		/**
		 * @param base Base directory, the data files are in
		 */
		DeleteDataFilesJob(const QString & base);
		virtual ~DeleteDataFilesJob();
		
		/**
		 * Add a file to delete
		 * @param file File
		 */
		void addFile(const QString & file);
		
		/**
		 * Check all directories in fpath and delete them if they are empty.
		 * The base path passed in the constructor, will be prepended to fpath.
		 * @param fpath The file path
		 */
		void addEmptyDirectoryCheck(const QString & fpath);
		
		/// Start the job
		virtual void start();
		
		/// Kill the job
		virtual void kill(bool quietly);
		
	private slots:
		void onDeleteJobDone(KJob* j);
		
	private:
		struct DirTree
		{
			QString name;
			bt::PtrMap<QString,DirTree> subdirs;
			
			DirTree(const QString & name);
			~DirTree();
			
			void insert(const QString & fpath);
			void doDeleteOnEmpty(const QString & base);
		};

	private:
		KUrl::List files;
		QString base;
		DirTree* directory_tree;
		KIO::Job* active_job;
	};

}

#endif
