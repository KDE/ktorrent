/*
    SPDX-FileCopyrightText: 2009 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "viewselectionmodel.h"
#include "viewmodel.h"
#include <util/log.h>

using namespace bt;

namespace kt
{
ViewSelectionModel::ViewSelectionModel(ViewModel *vm, QObject *parent)
    : ItemSelectionModel(vm, parent)
{
}

ViewSelectionModel::~ViewSelectionModel()
{
}
}
