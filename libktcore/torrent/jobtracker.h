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

#include <KJobTrackerInterface>

#include <ktcore_export.h>
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
        ~JobTracker();

        void registerJob(KJob* job) override;
        void unregisterJob(KJob* job) override;

        /// A job has been registered
        virtual void jobRegistered(bt::Job* j) = 0;

        /// A job has been unregistered
        virtual void jobUnregistered(bt::Job* j) = 0;

        /// Create a widget for a job
        virtual JobProgressWidget* createJobWidget(bt::Job* job);

    protected:
        void finished(KJob* job) override;
        void suspended(KJob* job) override;
        void resumed(KJob* job) override;
        void description(KJob* job, const QString& title, const QPair< QString, QString >& field1, const QPair< QString, QString >& field2) override;
        void infoMessage(KJob* job, const QString& plain, const QString& rich) override;
        void warning(KJob* job, const QString& plain, const QString& rich) override;
        void totalAmount(KJob* job, KJob::Unit unit, qulonglong amount) override;
        void processedAmount(KJob* job, KJob::Unit unit, qulonglong amount) override;
        void percent(KJob* job, long unsigned int percent) override;
        void speed(KJob* job, long unsigned int value) override;

    protected:
        typedef QMap<bt::Job*, JobProgressWidget*> ActiveJobs;
        ActiveJobs widgets;
    };

}

#endif // KT_JOBTRACKER_H
