/*
    SPDX-FileCopyrightText: 2005 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "prefpageinterface.h"

namespace kt
{
PrefPageInterface::PrefPageInterface(KConfigSkeleton *cfg, const QString &name, const QString &icon, QWidget *parent)
    : QWidget(parent)
    , cfg(cfg)
    , name(name)
    , icon(icon)
{
}

PrefPageInterface::~PrefPageInterface()
{
}

void PrefPageInterface::loadSettings()
{
}

void PrefPageInterface::loadDefaults()
{
}

void PrefPageInterface::updateSettings()
{
}
}
