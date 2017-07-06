/***************************************************************************
 *   Copyright (C) 2007 by Joris Guisson and Ivan Vasic                    *
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

#ifndef KTMISSINGFILESDLG_H
#define KTMISSINGFILESDLG_H

#include <QDialog>
#include "ui_missingfilesdlg.h"

namespace bt
{
    class TorrentInterface;
}

namespace kt
{

    /**
        Dialog to show when files are missing.
    */
    class MissingFilesDlg : public QDialog, public Ui_MissingFilesDlg
    {
        Q_OBJECT
    public:

        /**
         * Constructor
         * @param text Text to show above file list
         * @param missing The list of missing files
         * @param tc The torrent
         * @param parent The parent widget
         */
        MissingFilesDlg(const QString& text, const QStringList& missing, bt::TorrentInterface* tc, QWidget* parent);
        ~MissingFilesDlg();

        enum ReturnCode
        {
            RECREATE, DO_NOT_DOWNLOAD, CANCEL, NEW_LOCATION_SELECTED
        };

        /**
         * Execute the dialog
         * @return What to do
         */
        ReturnCode execute();

    private slots:
        void dndPressed();
        void recreatePressed();
        void cancelPressed();
        void selectNewPressed();

    private:
        ReturnCode ret;
        bt::TorrentInterface* tc;
    };

}

#endif
