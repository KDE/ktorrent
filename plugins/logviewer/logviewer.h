/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson                                   *
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
#ifndef KTLOGVIEWER_H
#define KTLOGVIEWER_H

#include <QMutex>
#include <QTextBrowser>

#include <interfaces/activity.h>
#include <interfaces/logmonitorinterface.h>
#include "logflags.h"


namespace kt
{
    /**
     * @author Joris Guisson
    */
    class LogViewer : public Activity, public bt::LogMonitorInterface
    {
        Q_OBJECT
    public:
        LogViewer(LogFlags* flags, QWidget* parent = 0);
        ~LogViewer();

        void message(const QString& line, unsigned int arg) override;

        void setRichText(bool val);
        void setMaxBlockCount(int max);
        void processPending();

    public slots:
        void showMenu(const QPoint& pos);
        void suspend(bool on);

    private:
        bool use_rich_text;
        LogFlags* flags;
        QTextBrowser* output;
        bool suspended;
        QMenu* menu;
        QAction* suspend_action;
        int max_block_count;

        QMutex mutex;
        QStringList pending;
    };

}

#endif
