/*
    SPDX-FileCopyrightText: 2005 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef KTLOGVIEWER_H
#define KTLOGVIEWER_H

#include <QMutex>
#include <QTextBrowser>

#include "logflags.h"
#include <interfaces/activity.h>
#include <interfaces/logmonitorinterface.h>

namespace kt
{
/**
 * @author Joris Guisson
 */
class LogViewer : public Activity, public bt::LogMonitorInterface
{
    Q_OBJECT
public:
    LogViewer(LogFlags *flags, QWidget *parent = nullptr);
    ~LogViewer() override;

    void message(const QString &line, unsigned int arg) override;

    void setRichText(bool val);
    void setMaxBlockCount(int max);
    void processPending();

public Q_SLOTS:
    void showMenu(const QPoint &pos);
    void suspend(bool on);

private:
    bool use_rich_text;
    LogFlags *flags;
    QTextBrowser *output;
    bool suspended;
    QMenu *menu;
    QAction *suspend_action;
    int max_block_count;

    QMutex mutex;
    QStringList pending;
};

}

#endif
