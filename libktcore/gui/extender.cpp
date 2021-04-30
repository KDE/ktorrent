/*
    SPDX-FileCopyrightText: 2010 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "extender.h"

namespace kt
{
Extender::Extender(bt::TorrentInterface *tc, QWidget *parent)
    : QWidget(parent)
    , tc(tc)
{
}

Extender::~Extender()
{
}
}
