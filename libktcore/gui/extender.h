/***************************************************************************
 *   Copyright (C) 2010 by Joris Guisson                                   *
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

#ifndef KT_EXTENDER_H
#define KT_EXTENDER_H

#include <QWidget>
#include <ktcore_export.h>

namespace bt
{
    class TorrentInterface;
}

namespace kt
{

    /**
     * Base class for all extender widgets
     */
    class KTCORE_EXPORT Extender : public QWidget
    {
        Q_OBJECT
    public:
        Extender(bt::TorrentInterface* tc, QWidget* parent);
        ~Extender();

        /// Get the torrent of this extender
        bt::TorrentInterface* torrent() {return tc;}

        /// Is this similar to another extender
        virtual bool similar(Extender* ext) const = 0;

    signals:
        /// Should be emitted by an extender when it wants to close itself
        void closeRequest(Extender* ext);

        /// Should be emitted when an extender is resized
        void resized(Extender* ext);

    protected:
        bt::TorrentInterface* tc;
    };

}

#endif // KT_EXTENDER_H
