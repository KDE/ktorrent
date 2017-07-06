/***************************************************************************
 *   Copyright (C) 2006 by Ivan Vasić                                      *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.           *
 ***************************************************************************/

#ifndef KTschedulerPLUGIN_H
#define KTschedulerPLUGIN_H

#include <QAction>
#include <QTimer>
#include <interfaces/plugin.h>
#include <interfaces/guiinterface.h>
#include "screensaver_interface.h"


class QString;


namespace kt
{
    class ScheduleEditor;
    class Schedule;
    class BWPrefPage;

    /**
     * @author Ivan Vasic <ivasic@gmail.com>
     * @brief KTorrent scheduler plugin.
     *
     */
    class BWSchedulerPlugin : public Plugin
    {
        Q_OBJECT
    public:
        BWSchedulerPlugin(QObject* parent, const QVariantList& args);
        ~BWSchedulerPlugin();

        void load() override;
        void unload() override;
        bool versionCheck(const QString& version) const override;

    public slots:
        void timerTriggered();
        void onLoaded(Schedule* ns);
        void colorsChanged();
        void screensaverActivated(bool on);
        void networkStatusChanged(bool online);

    private:
        void setNormalLimits();
        void restartTimer();

    private:
        QTimer m_timer;
        ScheduleEditor* m_editor;
        Schedule* m_schedule;
        BWPrefPage* m_pref;
        org::freedesktop::ScreenSaver* screensaver;
        bool screensaver_on;
    };

}

#endif
