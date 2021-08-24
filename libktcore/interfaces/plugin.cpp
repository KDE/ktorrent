/*
    SPDX-FileCopyrightText: 2005 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "plugin.h"

namespace kt
{
Plugin::Plugin(QObject *parent)
    : KParts::Plugin(parent)
{
}

Plugin::~Plugin()
{
}

void Plugin::guiUpdate()
{
}

void Plugin::shutdown(bt::WaitJob *)
{
}
}
