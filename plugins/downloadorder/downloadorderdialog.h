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

#ifndef KTDOWNLOADORDERDIALOG_H
#define KTDOWNLOADORDERDIALOG_H

#include <QDialog>
#include "ui_downloadorderwidget.h"

namespace bt
{
    class TorrentInterface;
}

namespace kt
{
    class DownloadOrderPlugin;
    class DownloadOrderModel;

    /**
        Dialog to manipulate the download order.
    */
    class DownloadOrderDialog : public QDialog, public Ui_DownloadOrderWidget
    {
        Q_OBJECT
    public:
        DownloadOrderDialog(DownloadOrderPlugin* plugin, bt::TorrentInterface* tor, QWidget* parent);
        ~DownloadOrderDialog();

    private slots:
        void commitDownloadOrder();
        void moveUp();
        void moveDown();
        void moveTop();
        void moveBottom();
        void itemSelectionChanged(const QItemSelection& new_sel, const QItemSelection& old_sel);
        void customOrderEnableToggled(bool on);
        void search(const QString& text);

    private:
        bt::TorrentInterface* tor;
        DownloadOrderPlugin* plugin;
        DownloadOrderModel* model;
    };

}

#endif
