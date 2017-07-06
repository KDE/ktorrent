/***************************************************************************
 *   Copyright (C) 2011 by Joris Guisson                                   *
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

#ifndef KT_TORRENTSEARCHBAR_H
#define KT_TORRENTSEARCHBAR_H

#include <QLineEdit>
#include <QToolButton>
#include <QWidget>

#include <KSharedConfig>

namespace kt
{
    class View;

    /**
     * Search bar widget for torrents.
     */
    class TorrentSearchBar : public QWidget
    {
        Q_OBJECT
    public:
        TorrentSearchBar(View* view, QWidget* parent);
        ~TorrentSearchBar();

        void loadState(KSharedConfigPtr cfg);
        void saveState(KSharedConfigPtr cfg);

    public slots:
        void showBar();
        void hideBar();

    signals:
        void filterBarHidden(QString str);
        void filterBarShown(QString str);

    protected:
        bool eventFilter(QObject* obj, QEvent* ev) override;

    private:
        QToolButton* hide_search_bar;
        QLineEdit* search_bar;
    };
}

#endif // KT_TORRENTSEARCHBAR_H
