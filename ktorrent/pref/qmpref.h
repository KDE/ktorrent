/***************************************************************************
 *   Copyright (C) 2009 by Joris Guisson                                   *
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

#ifndef KTQMPREF_H
#define KTQMPREF_H

#include <interfaces/prefpageinterface.h>
#include "ui_qmpref.h"

namespace kt
{

    /**
        Preference page for the queue manager
    */
    class QMPref : public PrefPageInterface, public Ui_QMPref
    {
        Q_OBJECT
    public:
        QMPref(QWidget* parent);
        ~QMPref();

        void loadSettings() override;
        void loadDefaults() override;
    private slots:
        void onControlTorrentsManuallyToggled(bool on);
    };

}

#endif
