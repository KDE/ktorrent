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
#ifndef KTEXITOPERATION_H
#define KTEXITOPERATION_H

#include <qobject.h>
#include <kio/job.h>

namespace kt
{

	/**
	 * @author Joris Guisson <joris.guisson@gmail.com>
	 * 
	 * Object to derive from for operations which need to be performed at exit.
	 * The operation should emit the operationFinished signal when they are done.
	 * 
	 * ExitOperation's can be used in combination with a WaitJob, to wait for a certain amount of time
	 * to give serveral ExitOperation's the time time to finish up.
	*/
	class ExitOperation : public QObject
	{
		Q_OBJECT
	public:
		ExitOperation();
		virtual ~ExitOperation();

		/// wether or not we can do a deleteLater on the job after it has finished.
		virtual bool deleteAllowed() const {return true;}
	signals:
		void operationFinished(kt::ExitOperation* opt);
	};

	/**
	 * Exit operation which waits for a KIO::Job
	 */
	class ExitJobOperation : public ExitOperation
	{
		Q_OBJECT
	public:
		ExitJobOperation(KIO::Job* j);
		virtual ~ExitJobOperation();
		
		virtual bool deleteAllowed() const {return false;}
	private slots:
		virtual void onResult(KIO::Job* j);
	};
}

#endif
