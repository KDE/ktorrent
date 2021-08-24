/*
    SPDX-FileCopyrightText: 2005 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "plugin.h"

namespace kt
{
Plugin::Plugin(QObject *parent, const KPluginMetaData &data, const QVariantList &args)
    : KParts::Plugin(parent)
{
    setMetaData(data);
    Q_UNUSED(args)
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
