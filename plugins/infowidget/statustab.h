/***************************************************************************
 *   Copyright (C) 2005-2007 by Joris Guisson                              *
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

#ifndef STATUSTAB_H
#define STATUSTAB_H

#include <QPointer>
#include <QWidget>
#include <interfaces/torrentinterface.h>
#include "ui_statustab.h"

namespace kt
{

    class StatusTab : public QWidget, public Ui_StatusTab
    {
        Q_OBJECT

    public:
        StatusTab(QWidget* parent);
        ~StatusTab();


    public slots:
        void changeTC(bt::TorrentInterface* tc);
        void update();
        void maxRatioChanged(double v);
        void useRatioLimitToggled(bool on);
        void useTimeLimitToggled(bool on);
        void maxTimeChanged(double v);
        void linkActivated(const QString& link);

    private:
        void maxRatioUpdate();
        void maxSeedTimeUpdate();

    private:
        QPointer<bt::TorrentInterface> curr_tc;
    };
}

#endif

