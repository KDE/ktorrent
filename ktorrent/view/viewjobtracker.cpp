/***************************************************************************
 *   Copyright (C) 2011 by Joris Guisson                                   *
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

#include "viewjobtracker.h"
#include "view.h"
#include "scanextender.h"
#include <torrent/torrentcontrol.h>
#include "viewdelegate.h"


namespace kt
{
    ViewJobTracker::ViewJobTracker(View* parent)
        : JobTracker(parent),
          view(parent)
    {
    }


    ViewJobTracker::~ViewJobTracker()
    {
    }

    void ViewJobTracker::jobUnregistered(bt::Job* j)
    {
        ActiveJobs::iterator i = widgets.find(j);
        if (i == widgets.end())
            return;

        JobProgressWidget* w = i.value();
        if (w->automaticRemove())
            w->emitCloseRequest();
    }

    void ViewJobTracker::jobRegistered(bt::Job* j)
    {
        kt::JobProgressWidget* widget = createJobWidget(j);
        view->extend(j->torrent(), widget, j->torrentStatus() == bt::CHECKING_DATA);
    }

    kt::JobProgressWidget* ViewJobTracker::createJobWidget(bt::Job* job)
    {
        if (job->torrentStatus() == bt::CHECKING_DATA)
        {
            ScanExtender* ext = new ScanExtender(job, 0);
            widgets[job] = ext;
            return ext;
        }
        else
            return kt::JobTracker::createJobWidget(job);
    }
}
