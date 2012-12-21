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

#include "jobtracker.h"
#include "basicjobprogresswidget.h"

namespace kt
{
    JobTracker::JobTracker(QObject* parent) : KJobTrackerInterface(parent)
    {
        bt::Job::setJobTracker(this);
    }

    JobTracker::~JobTracker()
    {
        bt::Job::setJobTracker(0);
    }

    void JobTracker::registerJob(KJob* job)
    {
        bt::Job* j = dynamic_cast<bt::Job*>(job);
        if (!j)
            return;

        KJobTrackerInterface::registerJob(job);
        jobRegistered(j);
    }

    void JobTracker::unregisterJob(KJob* job)
    {
        bt::Job* j = dynamic_cast<bt::Job*>(job);
        if (!j)
            return;

        KJobTrackerInterface::unregisterJob(job);
        jobUnregistered(j);
        widgets.remove(j);
    }

    JobProgressWidget* JobTracker::createJobWidget(bt::Job* job)
    {
        JobProgressWidget* p = new BasicJobProgressWidget(job, 0);
        widgets[job] = p;
        return p;
    }


    void JobTracker::finished(KJob* job)
    {
        KJobTrackerInterface::finished(job);
    }

    void JobTracker::suspended(KJob* job)
    {
        KJobTrackerInterface::suspended(job);
    }

    void JobTracker::resumed(KJob* job)
    {
        KJobTrackerInterface::resumed(job);
    }

    void JobTracker::description(KJob* job, const QString& title, const QPair< QString, QString >& field1, const QPair< QString, QString >& field2)
    {
        bt::Job* j = dynamic_cast<bt::Job*>(job);
        if (!j)
            return;

        ActiveJobs::iterator i = widgets.find(j);
        if (i != widgets.end())
            i.value()->description(title, field1, field2);
    }

    void JobTracker::infoMessage(KJob* job, const QString& plain, const QString& rich)
    {
        bt::Job* j = dynamic_cast<bt::Job*>(job);
        if (!j)
            return;

        ActiveJobs::iterator i = widgets.find(j);
        if (i != widgets.end())
            i.value()->infoMessage(plain, rich);
    }

    void JobTracker::warning(KJob* job, const QString& plain, const QString& rich)
    {
        bt::Job* j = dynamic_cast<bt::Job*>(job);
        if (!j)
            return;

        ActiveJobs::iterator i = widgets.find(j);
        if (i != widgets.end())
            i.value()->warning(plain, rich);
    }

    void JobTracker::totalAmount(KJob* job, KJob::Unit unit, qulonglong amount)
    {
        bt::Job* j = dynamic_cast<bt::Job*>(job);
        if (!j)
            return;

        ActiveJobs::iterator i = widgets.find(j);
        if (i != widgets.end())
            i.value()->totalAmount(unit, amount);
    }

    void JobTracker::processedAmount(KJob* job, KJob::Unit unit, qulonglong amount)
    {
        bt::Job* j = dynamic_cast<bt::Job*>(job);
        if (!j)
            return;

        ActiveJobs::iterator i = widgets.find(j);
        if (i != widgets.end())
            i.value()->processedAmount(unit, amount);
    }

    void JobTracker::percent(KJob* job, long unsigned int percent)
    {
        bt::Job* j = dynamic_cast<bt::Job*>(job);
        if (!j)
            return;

        ActiveJobs::iterator i = widgets.find(j);
        if (i != widgets.end())
            i.value()->percent(percent);
    }

    void JobTracker::speed(KJob* job, long unsigned int value)
    {
        bt::Job* j = dynamic_cast<bt::Job*>(job);
        if (!j)
            return;

        ActiveJobs::iterator i = widgets.find(j);
        if (i != widgets.end())
            i.value()->speed(value);
    }
}


