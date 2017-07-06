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

#ifndef KT_MAGNETVIEW_H
#define KT_MAGNETVIEW_H

#include <QWidget>
#include <KSharedConfig>

class QItemSelection;
class QToolBar;
class QTreeView;
class QMenu;

namespace kt
{
    class MagnetManager;
    class MagnetModel;

    /**
        View which displays a list of magnet links being downloaded
    */
    class MagnetView : public QWidget
    {
        Q_OBJECT
    public:
        MagnetView(MagnetManager* magnetManager, QWidget* parent = 0);
        ~MagnetView();

        void saveState(KSharedConfigPtr cfg);
        void loadState(KSharedConfigPtr cfg);

        void keyPressEvent(QKeyEvent* event) override;

    private slots:
        void showContextMenu(QPoint p);
        void removeMagnetDownload();
        void startMagnetDownload();
        void stopMagnetDownload();

    private:
        MagnetManager* mman;
        MagnetModel* model;
        QTreeView* view;
        QMenu* menu;

        QAction* start;
        QAction* stop;
        QAction* remove;
    };

}

#endif // KT_MAGNETVIEW_H
