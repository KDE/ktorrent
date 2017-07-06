/***************************************************************************
 *   Copyright (C) 2008 by Joris Guisson and Ivan Vasic                    *
 *   joris.guisson@gmail.com                                               *
 *   ivasic@gmail.com                                                      *
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

#ifndef KTRECOMMENDEDSETTINGSDLG_H
#define KTRECOMMENDEDSETTINGSDLG_H

#include <QDialog>
#include <util/constants.h>
#include "ui_recommendedsettingsdlg.h"

namespace kt
{

    /**
        Dialog to compute the best settings
    */
    class RecommendedSettingsDlg : public QDialog, public Ui_RecommendedSettingsDlg
    {
        Q_OBJECT
    public:
        RecommendedSettingsDlg(QWidget* parent);
        ~RecommendedSettingsDlg();

    private slots:
        void calculate();
        void apply();
        void avgSpeedSlotToggled(bool on);
        void simTorrentsToggled(bool on);
        void slotsToggled(bool on);
        void uploadBWChanged(int val);
        void downloadBWChanged(int val);

    private:
        void saveState(KSharedConfigPtr cfg);
        void loadState(KSharedConfigPtr cfg);

    public:
        bt::Uint32 max_upload_speed;
        bt::Uint32 max_download_speed;
        bt::Uint32 max_conn_tor;
        bt::Uint32 max_conn_glob;
        bt::Uint32 max_downloads;
        bt::Uint32 max_seeds;
        bt::Uint32 max_slots;
    };

}

#endif
