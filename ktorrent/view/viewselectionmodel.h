/*
    SPDX-FileCopyrightText: 2009 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTVIEWSELECTIONMODEL_H
#define KTVIEWSELECTIONMODEL_H

#include <util/itemselectionmodel.h>

namespace bt
{
}

namespace kt
{
class ViewModel;

/**
    Custom selection model for View
*/
class ViewSelectionModel : public ItemSelectionModel
{
    Q_OBJECT
public:
    ViewSelectionModel(ViewModel *vm, QObject *parent);
    ~ViewSelectionModel() override;
};

}

#endif
