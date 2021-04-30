/*
    SPDX-FileCopyrightText: 2010 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "jobprogresswidget.h"
#include <torrent/torrentcontrol.h>

namespace kt
{
JobProgressWidget::JobProgressWidget(bt::Job *job, QWidget *parent)
    : Extender(job->torrent(), parent)
    , job(job)
    , automatic_remove(true)
{
}

JobProgressWidget::~JobProgressWidget()
{
}

void JobProgressWidget::emitCloseRequest()
{
    closeRequest(this);
}

}
