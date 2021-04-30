/*
    SPDX-FileCopyrightText: 2011 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "viewjobtracker.h"
#include "scanextender.h"
#include "view.h"
#include "viewdelegate.h"
#include <torrent/torrentcontrol.h>

namespace kt
{
ViewJobTracker::ViewJobTracker(View *parent)
    : JobTracker(parent)
    , view(parent)
{
}

ViewJobTracker::~ViewJobTracker()
{
}

void ViewJobTracker::jobUnregistered(bt::Job *j)
{
    ActiveJobs::iterator i = widgets.find(j);
    if (i == widgets.end())
        return;

    JobProgressWidget *w = i.value();
    if (w->automaticRemove())
        w->emitCloseRequest();
}

void ViewJobTracker::jobRegistered(bt::Job *j)
{
    kt::JobProgressWidget *widget = createJobWidget(j);
    view->extend(j->torrent(), widget, j->torrentStatus() == bt::CHECKING_DATA);
}

kt::JobProgressWidget *ViewJobTracker::createJobWidget(bt::Job *job)
{
    if (job->torrentStatus() == bt::CHECKING_DATA) {
        ScanExtender *ext = new ScanExtender(job, 0);
        widgets[job] = ext;
        return ext;
    } else
        return kt::JobTracker::createJobWidget(job);
}
}
