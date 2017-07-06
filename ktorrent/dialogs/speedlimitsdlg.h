/***************************************************************************
 *   Copyright (C) 2007 by Joris Guisson                                   *
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

#ifndef SPEEDLIMITSDLG_H
#define SPEEDLIMITSDLG_H

#include <QDialog>
#include "ui_speedlimitsdlg.h"

namespace bt
{
    class TorrentInterface;
}

namespace kt
{
    class Core;
    class SpeedLimitsModel;

    /// Dialog to modify the speed limits of a torrent
    class SpeedLimitsDlg : public QDialog, public Ui_SpeedLimitsDlg
    {
        Q_OBJECT

    public:
        SpeedLimitsDlg(bt::TorrentInterface* current, Core* core, QWidget* parent);
        ~SpeedLimitsDlg();


    protected slots:
        void accept() override;
        void reject() override;
        void apply();
        void spinBoxValueChanged(int);
        void saveState();
        void loadState();

    private:
        Core* core;
        SpeedLimitsModel* model;
        bt::TorrentInterface* current;
    };
}

#endif

