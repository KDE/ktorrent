/*
    SPDX-FileCopyrightText: 2011 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KT_VIEWJOBTRACKER_H
#define KT_VIEWJOBTRACKER_H

#include <torrent/jobtracker.h>

namespace kt
{
class View;

/**
    JobTracker for the View
 */
class ViewJobTracker : public kt::JobTracker
{
    Q_OBJECT
public:
    ViewJobTracker(View *parent);
    ~ViewJobTracker() override;

    void jobUnregistered(bt::Job *j) override;
    void jobRegistered(bt::Job *j) override;
    kt::JobProgressWidget *createJobWidget(bt::Job *job) override;

private:
    View *view;
};

}

#endif // KT_VIEWJOBTRACKER_H
