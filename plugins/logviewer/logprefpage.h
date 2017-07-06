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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/

#ifndef KTLOGPREFPAGE_H
#define KTLOGPREFPAGE_H

#include <interfaces/prefpageinterface.h>
#include "ui_logprefwidget.h"

namespace kt
{
    class LogFlags;

    class LogPrefPage : public PrefPageInterface, public Ui_LogPrefWidget
    {
        Q_OBJECT
    public:
        LogPrefPage(LogFlags* flags, QWidget* parent);
        ~LogPrefPage();

        void loadDefaults() override;
        void loadSettings() override;
        void updateSettings() override;

        void saveState();
        void loadState();
    private:
        bool state_loaded;
    };
}

#endif
