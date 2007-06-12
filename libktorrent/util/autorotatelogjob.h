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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/
#ifndef BTAUTOROTATELOGJOB_H
#define BTAUTOROTATELOGJOB_H

#include <kio/job.h>
#include <cstdlib>

namespace bt
{
	class Log;

	/**
		@author Joris Guisson <joris.guisson@gmail.com>
		
		Job which handles the rotation of the log file. 
		This Job must do several move jobs which must be done sequentially.
	*/
	class AutoRotateLogJob : public KIO::Job
	{
		Q_OBJECT
	public:
		AutoRotateLogJob(const QString & file,Log* lg);
		virtual ~AutoRotateLogJob();
		
		virtual void kill(bool quietly=true);
		
	private slots:
		void moveJobDone(KIO::Job*);
		
	private:
		void update();
		
	private:
		QString file;
		int cnt;
		Log* lg;
	};

}

#endif
