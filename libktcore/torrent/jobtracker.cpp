/*
    SPDX-FileCopyrightText: 2010 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "jobtracker.h"
#include "basicjobprogresswidget.h"

namespace kt
{
JobTracker::JobTracker(QObject *parent)
    : KJobTrackerInterface(parent)
{
    bt::Job::setJobTracker(this);
}

JobTracker::~JobTracker()
{
    bt::Job::setJobTracker(0);
}

void JobTracker::registerJob(KJob *job)
{
    bt::Job *j = dynamic_cast<bt::Job *>(job);
    if (!j)
        return;

    KJobTrackerInterface::registerJob(job);
    jobRegistered(j);
}

void JobTracker::unregisterJob(KJob *job)
{
    bt::Job *j = dynamic_cast<bt::Job *>(job);
    if (!j)
        return;

    KJobTrackerInterface::unregisterJob(job);
    jobUnregistered(j);
    widgets.remove(j);
}

JobProgressWidget *JobTracker::createJobWidget(bt::Job *job)
{
    JobProgressWidget *p = new BasicJobProgressWidget(job, 0);
    widgets[job] = p;
    return p;
}

void JobTracker::finished(KJob *job)
{
    KJobTrackerInterface::finished(job);
}

void JobTracker::suspended(KJob *job)
{
    KJobTrackerInterface::suspended(job);
}

void JobTracker::resumed(KJob *job)
{
    KJobTrackerInterface::resumed(job);
}

void JobTracker::description(KJob *job, const QString &title, const QPair<QString, QString> &field1, const QPair<QString, QString> &field2)
{
    bt::Job *j = dynamic_cast<bt::Job *>(job);
    if (!j)
        return;

    ActiveJobs::iterator i = widgets.find(j);
    if (i != widgets.end())
        i.value()->description(title, field1, field2);
}

void JobTracker::infoMessage(KJob *job, const QString &plain, const QString &rich)
{
    bt::Job *j = dynamic_cast<bt::Job *>(job);
    if (!j)
        return;

    ActiveJobs::iterator i = widgets.find(j);
    if (i != widgets.end())
        i.value()->infoMessage(plain, rich);
}

void JobTracker::warning(KJob *job, const QString &plain, const QString &rich)
{
    bt::Job *j = dynamic_cast<bt::Job *>(job);
    if (!j)
        return;

    ActiveJobs::iterator i = widgets.find(j);
    if (i != widgets.end())
        i.value()->warning(plain, rich);
}

void JobTracker::totalAmount(KJob *job, KJob::Unit unit, qulonglong amount)
{
    bt::Job *j = dynamic_cast<bt::Job *>(job);
    if (!j)
        return;

    ActiveJobs::iterator i = widgets.find(j);
    if (i != widgets.end())
        i.value()->totalAmount(unit, amount);
}

void JobTracker::processedAmount(KJob *job, KJob::Unit unit, qulonglong amount)
{
    bt::Job *j = dynamic_cast<bt::Job *>(job);
    if (!j)
        return;

    ActiveJobs::iterator i = widgets.find(j);
    if (i != widgets.end())
        i.value()->processedAmount(unit, amount);
}

void JobTracker::percent(KJob *job, long unsigned int percent)
{
    bt::Job *j = dynamic_cast<bt::Job *>(job);
    if (!j)
        return;

    ActiveJobs::iterator i = widgets.find(j);
    if (i != widgets.end())
        i.value()->percent(percent);
}

void JobTracker::speed(KJob *job, long unsigned int value)
{
    bt::Job *j = dynamic_cast<bt::Job *>(job);
    if (!j)
        return;

    ActiveJobs::iterator i = widgets.find(j);
    if (i != widgets.end())
        i.value()->speed(value);
}
}
