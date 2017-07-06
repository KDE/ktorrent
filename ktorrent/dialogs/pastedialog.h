/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson                                   *
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

#ifndef PASTEDIALOG_H
#define PASTEDIALOG_H

#include <QDialog>
#include <KSharedConfig>
#include "ui_pastedlgbase.h"


namespace kt
{
    class Core;

    /**
     * @author Ivan Vasic
     * @brief Torrent URL paste dialog
     **/
    class PasteDialog: public QDialog, public Ui_PasteDlgBase
    {
        Q_OBJECT
    public:
        PasteDialog(Core* core, QWidget* parent = 0, Qt::WindowFlags fl = 0);
        ~PasteDialog();

        /**
        * Load the state of the dialog
        */
        void loadState(KSharedConfig::Ptr cfg);

        /**
        * Save the state of the dialog
        */
        void saveState(KSharedConfig::Ptr cfg);

    public slots:
        void accept() override;

    private:
        void loadGroups();

    private:
        Core* m_core;
    };
}
#endif
