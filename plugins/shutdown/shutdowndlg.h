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

#ifndef KT_SHUTDOWNDLG_H
#define KT_SHUTDOWNDLG_H

#include <QDialog>
#include "ui_shutdowndlg.h"
#include "shutdownruleset.h"

namespace kt
{
    class CoreInterface;
    class ShutdownTorrentModel;


    class ShutdownDlg : public QDialog, public Ui_ShutdownDlg
    {
        Q_OBJECT
    public:
        ShutdownDlg(ShutdownRuleSet* rules, CoreInterface* core, QWidget* parent);
        ~ShutdownDlg();

        void accept() override;
        void reject() override;

    private slots:
        void timeToExecuteChanged(int idx);

    private:
        Action indexToAction(int idx);
        int actionToIndex(Action act);

    private:
        ShutdownRuleSet* rules;
        ShutdownTorrentModel* model;
    };

}

#endif // KT_SHUTDOWNDLG_H
