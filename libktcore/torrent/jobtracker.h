/***************************************************************************
 *   Copyright (C) 2010 by Joris Guisson                                   *
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


#ifndef KT_JOBTRACKER_H
#define KT_JOBTRACKER_H

#include <ktcore_export.h>
#include <kjobtrackerinterface.h>
#include <torrent/job.h>


namespace kt 
{
	class JobProgressWidget;
	
	/**
		JobTracker for bt::Job's
	 */
	class KTCORE_EXPORT JobTracker : public KJobTrackerInterface
	{
		Q_OBJECT
	public:
		JobTracker(QObject* parent);
		virtual ~JobTracker();
		
		virtual void registerJob(KJob* job);
		virtual void unregisterJob(KJob* job);
		
		/// A job has been registered
		virtual void jobRegistered(bt::Job* j) = 0;
		
		/// A job has been unregistered
		virtual void jobUnregistered(bt::Job* j) = 0;
		
		/// Create a widget for a job
		virtual JobProgressWidget* createJobWidget(bt::Job* job);

	protected:
		virtual void finished(KJob* job);
		virtual void suspended(KJob* job);
		virtual void resumed(KJob* job);
		virtual void description(KJob* job, const QString& title, const QPair< QString, QString >& field1, const QPair< QString, QString >& field2);
		virtual void infoMessage(KJob* job, const QString& plain, const QString& rich);
		virtual void warning(KJob* job, const QString& plain, const QString& rich);
		virtual void totalAmount(KJob* job, KJob::Unit unit, qulonglong amount);
		virtual void processedAmount(KJob* job, KJob::Unit unit, qulonglong amount);
		virtual void percent(KJob* job, long unsigned int percent);
		virtual void speed(KJob* job, long unsigned int value);

	protected:
		typedef QList<JobProgressWidget*> JobProgessWidgetList;
		typedef QMap<bt::Job*,JobProgessWidgetList> ActiveJobs;
		ActiveJobs widgets;
	};

}

#endif // KT_JOBTRACKER_H
