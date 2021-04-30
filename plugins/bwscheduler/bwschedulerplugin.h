/*
    SPDX-FileCopyrightText: 2006 Ivan Vasić <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTschedulerPLUGIN_H
#define KTschedulerPLUGIN_H

#include "screensaver_interface.h"
#include <QAction>
#include <QTimer>
#include <interfaces/guiinterface.h>
#include <interfaces/plugin.h>

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
    BWSchedulerPlugin(QObject *parent, const QVariantList &args);
    ~BWSchedulerPlugin() override;

    void load() override;
    void unload() override;
    bool versionCheck(const QString &version) const override;

public Q_SLOTS:
    void timerTriggered();
    void onLoaded(Schedule *ns);
    void colorsChanged();
    void screensaverActivated(bool on);
    void networkStatusChanged(bool online);

private:
    void setNormalLimits();
    void restartTimer();

private:
    QTimer m_timer;
    ScheduleEditor *m_editor;
    Schedule *m_schedule;
    BWPrefPage *m_pref;
    org::freedesktop::ScreenSaver *screensaver;
    bool screensaver_on;
};

}

#endif
